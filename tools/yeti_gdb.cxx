/*
 * enable protocol debug
 * (gdb) set debug remote 1
 * set arch
 * (gdb) set architecture riscv:rv32
 * compressed code is not supported
 * (gdb) set riscv use-compressed-breakpoints off
 * connect to vm
 * (gdb) target remote :4321
 */

#include <asio.hpp>

#include <iostream>
#include <format>
#include <string>
#include <vector>
#include <array>
#include <cstdint>
#include <bit>

#include <yeti-vm/vm_opcode.hxx>
#include <yeti-vm/vm_utility.hxx>

using asio::ip::tcp;

// https://sourceware.org/gdb/current/onlinedocs/gdb.html/Packets.html#Packets
// https://ftp.gnu.org/old-gnu/Manuals/gdb/html_node/gdb_129.html
namespace gdb_remote
{
    enum Command: char
    {
        /// Indicate the reason the target halted.
        LAST_SIGNAL = '?',
        /// (?)
        CONTINUE_c = 'c', // continue
        /// (?)
        CONTINUE_C = 'C', // continue with signal
        /// (?)
        STEP_s = 's', // single step
        /// (?)
        STEP_S = 'S', // step with signal
        /// stop debugging
        DETACH = 'D',
        /// get all registers
        GP_REG_GET = 'g',
        /// set registers
        GP_REG_SET = 'G',
        /// stop execution
        KILL_TGT = 'k',
        /// read memory: m<addr>,<size>
        /// reply: 'HH...'(success, hex data) or 'ENN'(error, NN - hex digits)
        MEM_GET = 'm',
        /// write memory: m<addr>,<size>:data
        /// reply: 'OK'(success) or 'ENN'(error, NN - hex digits)
        MEM_SET = 'M',
        /// read memory, binary
        MEM_BIN_GET = 'x',
        /// write memory, binary
        MEM_BIN_SET = 'X',
        /// get register pHH
        /// note: value encoded in target byte order
        /// return 'xx'*sizeof(reg) if register not available
        REG_GET = 'p',
        /// set register PHH=hh...
        /// note: value encoded in target byte order
        REG_SET = 'P',
        /// generic query, get value
        GENERIC_Q_GET = 'q',
        /// generic query, set value
        GENERIC_Q_SET = 'Q',
        /// multi-letter query
        QUERY_V = 'v',
        /// remove breakpoint/watchpoint: z<type>,<addr>,<length>
        BREAK_CLR = 'z',
        /// insert breakpoint/watchpoint: Z<type>,<addr>,<length>
        BREAK_SET = 'Z',
        /// H<cmd><thread> set current thread for command
        THREAD_SET = 'H',
    };

    enum Protocol: char
    {
        /// checksum ok
        GDB_ACK = '+',
        /// checksum error
        GDB_NAK = '-',
        /// cmd start mark
        GDB_COMMAND = '$',
        /// notify start mark
        GDB_NOTIFY = '%',
        /// data end mark
        GDB_END = '#',
        /// next char is escaped and XOR-ed with 0x20
        GDB_ESC = '}',
        /// encode/decode escaped char: c = c ^ 0x20
        GDB_XOR = 0x20, // just ' ' - space
        /// RLE encoded, next char - length
        GDB_RLE_MARK = '*',
        /// rle_length = c - 28
        /// in gdb source: repeat = c - ' ' + 3
        GDB_RLE_LENGTH = 28,
        /// ctr+C - user request interrupt
        GDB_BREAK = 0x03,
    };
} // namespace gdb_remote

int main(int argc, char ** argv)
{
    using namespace vm::opcode;
    asio::io_context ctx;

    // note: gdb assumes that PC is regs[32]. how to change this?
    std::array<uint32_t, 32 + 1> regs{}; // registers: GP + PC
    std::vector<uint8_t> ram;
    ram.resize(0x8000000, 0); // 8MiB

    // fill mem with NOP
    auto mem_code = std::span(reinterpret_cast<uint32_t*>(ram.data()), 16);
    for (auto& c: mem_code)
    {
        c = Encoder::i_type(OP_IMM, 0, 0, 0, 0); // nop; // addi x0, x0, 0
    }
    // jump to start
    mem_code.back() = Encoder::i_type(JALR, 0, 0, 0, 0); // jalr x0, 0(x0);

    tcp::acceptor server(ctx, tcp::endpoint(tcp::v4(), 4321));

    tcp::socket client(ctx);
    std::cout << "wait for connection" << std::endl;
    server.accept(client);
    std::cout << "new connection" << std::endl;
    try
    {
        tcp::iostream stream;
        bool run = true;
        static auto calc_crc = [](const std::string& data)
        {
            uint8_t sum = 0;
            for (uint8_t c: data)
                sum += c;
            return sum;
        };
        static auto reply = [](const std::string& data)
        {
            uint8_t sum = calc_crc(data);
            return std::format("${}#{:02X}", data, sum);
        };

        static auto notify = [](const std::string& data)
        {
            uint8_t sum = calc_crc(data);
            return std::format("%{}#{:02X}", data, sum);
        };

        static auto accepted = []()
        {
            return std::string{gdb_remote::GDB_ACK};
        };

        static auto rejected = []()
        {
            return std::string{gdb_remote::GDB_NAK};
        };

        static auto encode_rle = [](char c, char cnt)
        {
            return std::format("{}*{}", c, char(cnt + 28));
        };

        static auto encode_reg = [](uint32_t r)
        {
            auto v = reinterpret_cast<const uint8_t*>(&r);
            return std::format("{:02X}{:02X}{:02X}{:02X}", v[0], v[1], v[2], v[3]);
        };

        int state = 0;
        do
        {
            ++state;
            using namespace gdb_remote;
            char buff[1], crc_buf[2];
            std::string input, output;

            do
            {
                bool ctrl_c = false;
                asio::read(client, asio::buffer(buff));
                std::cout << std::format(">>{:02X}[{}]", buff[0], (std::isprint(buff[0]) ? buff[0]: '?') ) << std::endl;
                ctrl_c = buff[0] == GDB_BREAK;
                if (ctrl_c) // user request stop
                {
                    std::cout << "ctrl+c: SIGINT" << std::endl;
                    asio::write(client, asio::buffer(accepted()));
                    //asio::write(client, asio::buffer(notify("Stop:S02"))); // SIGINT
                    asio::write(client, asio::buffer(reply("S02"))); // SIGINT
                    continue;
                }
            } while (buff[0]!= GDB_COMMAND);


            bool esc = false;
            uint8_t crc_c = 0;
            while (true)
            {
                asio::read(client, asio::buffer(buff));
                if (buff[0] == GDB_END)
                    break;
                crc_c += buff[0];
                if (esc)
                {
                    buff[0] ^= Protocol::GDB_XOR;
                    esc = false;
                }
                else if (buff[0] == Protocol::GDB_ESC)
                {
                    esc = true;
                    continue;
                }
                input += buff[0];
            }

            asio::read(client, asio::buffer(crc_buf));
            std::string_view crc_view{crc_buf, 2};

            std::string_view cmd = input;
            uint8_t crc_i = (vm::from_hex(crc_buf[0]) << 4) | (vm::from_hex(crc_buf[1]) << 0);
            bool crc_ok = crc_c == crc_i;

            if (!crc_ok)
            {
                asio::write(client, asio::buffer(rejected()));
                continue;
            }
            asio::write(client, asio::buffer(accepted()));

            std::cout << std::format("{:04} -> [${}#{}][ok={}]: {}",  state, input, crc_view, crc_ok,  cmd) << std::endl;
            if (cmd.empty())
                continue;
            std::string args{cmd.substr(1)};
            switch (cmd[0])
            {
                case GENERIC_Q_GET:
                {
                    std::cout << "GENERIC_Q_GET" << std::endl;
                    if (cmd.starts_with("qSupported:")) // qSupported (supported-packets)
                        output = ("PacketSize=1000;hwbreak+;swbreak+;vContSupported+;xmlRegisters=rv32i"); // 'PacketSize' is required
                    else if (cmd == "qOffsets")
                    {
                        // gdb supports two formats:
                        // 0. empty response - command is not supported
                        // 1. Sections: Text=TTT;Data=DDD;Bss=BBB
                        //      Data must be equal Bss
                        //      All fields required
                        // 2. Segments: TextSeg=TTT[;DataSeg=DDD]
                        //      DataSeg can be omitted
                        // field values encoded in hex
                        output = ("Text=0;Data=400000;Bss=400000"); // Note: hex values
                    }
//                    else if (cmd.starts_with("qTStatus"))
//                        output = make_ack("");
//                    else if (cmd.starts_with("qSymbol:"))
//                        output = make_ack("OK");
//                    else if (cmd.starts_with("qfThreadInfo")) // threads info / replacement for 'qL'
//                        output = make_ack("l"); // no threads
//                    else if (cmd == "qAttached")
//                        output = make_ack("");
//                    else if (cmd == "qC")
//                        output = make_ack("-1");
//                    else
//                        output = make_ack(""); // should return "$#00' if command is unknown
                    break;
                }
                case QUERY_V: // not supported
                {
                    std::cout << "QUERY_V: " << args << std::endl;
                    if (cmd == "vCont?") // vCont (verbose-resume)
                        output = ("vCont;c;C;s;S"); // both 'c'/'C' is required
                    else if (cmd.starts_with("vCont;"))
                        output = ("OK"); // NOLINT(bugprone-branch-clone)
                    else if (cmd.starts_with("vCtrlC"))
                        output = ("OK"); // NOLINT(bugprone-branch-clone)
//                    else if (cmd.starts_with("vKill"))
//                        output = make_ack("OK");
//                    else
//                        output = make_ack("");
                    break;
                }
                case LAST_SIGNAL:
                {
                    std::cout << "LAST_SIGNAL(Why stopped?)" << std::endl;
                    // stop cause:
                    // SAA - signal AA received
                    // WAA - exit with code AA
                    // XAA - terminated with AA signal
                    output = ("S05"); // SIGTRAP
                    //output = ("W00"); // normal exit
                    break;
                }
                case THREAD_SET:
                {
                    std::cout << "THREAD_SET" << std::endl;
                    //output = (""); // no threads
                    break;
                }
                case GP_REG_GET: // get all GP registers
                {
                    std::cout << "GP_REG_GET" << std::endl;
                    std::string rx;
                    for (auto r: regs)
                    {
                        rx += encode_reg(r);
                    }
                    output = (rx); // note: encoded in target byteorder
                    break;
                }
                case GP_REG_SET: // set GP registers
                {
                    std::cout << "GP_REG_SET" << std::endl;
                    output = ("E01");
                    break;
                }
                case REG_GET: // pHH - get register 0xHH
                {
                    auto rid = std::stoul(args, nullptr, 16);
                    std::cout << "REG_GET: " << rid << std::endl;
                    if (rid < regs.size())
                    {
                        output = (encode_reg(regs[rid])); // note: encoded in target byteorder
                    }
                    else
                    {
                        output = ("E02"); // value not available
                    }
                    break;
                }
                case REG_SET: // pHH=value - set register 0xHH
                {
                    size_t idx = 0;
                    auto rid = std::stoul(args, &idx, 16);
                    // assume args[idx] == '='
                    auto val = vm::from_hex(args.substr(idx + 1));
                    std::cout << "REG_SET: "
                        << rid << " := " << val.size() << std::endl;
                    if (rid < regs.size() && val.size() == 4)
                    {
                        // note: encoded in target byteorder
                        regs[rid] = *reinterpret_cast<const uint32_t *>(val.data());
                        output = ("OK");
                    }
                    else
                    {
                        output = ("E02");
                    }
                    break;
                }
                case MEM_GET: // mADR,SZ - read memory
                {
                    size_t idx_addr = 0;
                    size_t idx_size = 0;
                    size_t addr = std::stoul(args, &idx_addr, 16);
                    size_t size = std::stoul(args.substr(idx_addr + 1), &idx_size, 16);
                    std::cout << std::format("MEM_GET: <{:08X}:{}>", addr, size) << std::endl;
                    if ((addr + size) >= ram.size())
                    {
                        output = ("E03"); // value not available
                    }
                    else
                    {
                        std::string res;
                        res.reserve(size * 2);
                        for (auto b: std::span(ram.data() + addr, size))
                        {
                            res += std::format("{:02X}", b);
                        }
                        output = (res);
                    }
                    break;
                }
                case MEM_SET: // mADR,SZ:data - write memory
                {
                    size_t idx_addr = 0;
                    size_t idx_size = 0;
                    size_t addr = std::stoul(args, &idx_addr, 16);
                    size_t size = std::stoul(args.substr(idx_addr + 1), &idx_size, 16);
                    std::string_view args_view = args;
                    std::string_view data_view = args_view.substr( idx_addr + 1 + idx_size + 1);
                    std::cout << std::format("MEM_SET: <{:08X}:{}> = [{}]", addr, size, data_view) << std::endl;
                    if ((addr + size) >= ram.size())
                    {
                        output = ("E04"); // out of range
                    }
                    else
                    {
                        auto data_raw = vm::from_hex(data_view);
                        if (data_view.size() == (data_raw.size() * 2))
                        {
                            std::copy_n(data_raw.data(), data_raw.size(), ram.data() + addr);
                            output = ("OK");
                        }
                        else
                        {
                            output = ("E05"); // data length error
                        }
                    }
                    break;
                }
                case MEM_BIN_GET:
                {
                    std::cout << "MEM_BIN_GET: " << args << std::endl;
                    break;
                }
                case MEM_BIN_SET:
                {
                    std::cout << "MEM_BIN_SET: " << args << std::endl;
                    break;
                }
                case BREAK_CLR:
                case BREAK_SET:
                {
                    std::cout << "BREAKPOINT: " << args << std::endl;
                    break;
                }
                case STEP_s:
                case STEP_S:
                {
                    std::cout << "STEP: " << args << std::endl;
                    break;
                }
                case CONTINUE_C:
                {
                    std::cout << "CONTINUE: " << args << std::endl;
                    break;
                }
                case CONTINUE_c: // exec until next stop
                {
                    std::cout << "CONTINUE" << std::endl;
                    output = "OK";
                    // should send 'stop' notify later
                    break;
                }
                case DETACH: // debugger detached, exit
                    std::cout << "DETACH" << std::endl;
                    output = ("OK");
                    run = false;
                    break;
                case KILL_TGT: // stop execution, exit
                    std::cout << "KILL_TGT" << std::endl;
                    output = "OK";
                    run = false;
                    break;
                default: // "unknown command"
                    std::cout << "[WARN]: unknown command, ignored" << std::endl;
                    break;
            }
            {
                output = reply(output);
                std::cout << std::format("{:04} <- [{}]", state, output) << std::endl;
                asio::write(client, asio::buffer(output));
            }
        } while (run);
    }
    catch (...)
    {}
    return 0;
}
