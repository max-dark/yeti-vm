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

using asio::ip::tcp;

/*
+$qSupported:multiprocess+;swbreak+;hwbreak+;qRelocInsn+;fork-events+;vfork-events+;exec-events+;vContSupported+;QThreadEvents+;QThreadOptions+;no-resumed+;memory-tagging+;xmlRegisters=i386#72
+$vCont?#49
+$vMustReplyEmpty#3a
$vMustReplyEmpty#3a
+$vMustReplyEmpty#3a
+$vMustReplyEmpty#3a
++$Hg0#df
$Hg0#df
+$Hg0#df
+$Hg0#df
++$qTStatus#49
$qTStatus#49
+$qTStatus#49
+$qTStatus#49
++$?#3f
$?#3f
+$?#3f
+$?#3f
++$qfThreadInfo#bb
$qfThreadInfo#bb
+$qfThreadInfo#bb
+$qfThreadInfo#bb
++$qL1200000000000000000#50
$qL1200000000000000000#50
+$qL1200000000000000000#50
+$qL1200000000000000000#50
++$Hc-1#09
$Hc-1#09
+$Hc-1#09
+$Hc-1#09
++$qC#b4
$qC#b4
+$qC#b4
+$qC#b4
++$qAttached#8f
$qAttached#8f
+$qAttached#8f
+$qAttached#8f

 */

/*
---------------------------------------------------
wait for connection
1:+$qSupported:multiprocess+;swbreak+;hwbreak+;qRelocInsn+;fork-events+;vfork-events+;exec-events+;vContSupported+;QThreadEvents+;QThreadOptions+;no-resumed+;memory-tagging+;xmlRegisters=i386#72
1-> [+$qSupported:multiprocess+;swbreak+;hwbreak+;qRelocInsn+;fork-events+;vfork-events+;exec-events+;vContSupported+;QThreadEvents+;QThreadOptions+;no-resumed+;memory-tagging+;xmlRegisters=i386#72]:qSupported:multiprocess+;swbreak+;hwbreak+;qRelocInsn+;fork-events+;vfork-events+;exec-events+;vContSupported+;QThreadEvents+;QThreadOptions+;no-resumed+;memory-tagging+;xmlRegisters=i386
1<- [+$#00]
2-> [+$vCont?#49]:vCont?
2<- [+$#00]
3-> [+$vMustReplyEmpty#3a]:vMustReplyEmpty
3<- [+$#00]
4-> [+$Hg0#df]:Hg0
4<- [+$#00]
5-> [+$qTStatus#49]:qTStatus
5<- [+$#00]
6-> [+$?#3f]:?
6<- [+$S05#B8]
7-> [+$qfThreadInfo#bb]:qfThreadInfo
7<- [+$#00]
8-> [+$qL1160000000000000000#55]:qL1160000000000000000
8<- [+$#00]
9-> [+$Hc-1#09]:Hc-1
9<- [+$#00]
10-> [+$qC#b4]:qC
10<- [+$#00]
11-> [+$qAttached#8f]:qAttached
11<- [+$#00]
12-> [+$qOffsets#4b]:qOffsets
12<- [+$#00]
13-> [+$g#67]:g
13<- [+$0000000000000000000000000000000000000000000000000000000000000000#00]
14-> [+$p20#d2]:p20
14<- [+$00000000#80]
15-> [+$qL1160000000000000000#55]:qL1160000000000000000
15<- [+$#00]
16-> [+$m0,4#fd]:m0,4
16<- [+$00000000#80]
17-> [+$mfffffffc,4#fa]:mfffffffc,4
17<- [+$00000000#80]
18-> [+$qSymbol::#5b]:qSymbol::
18<- [+$#00]
19-> [+$vKill;a410#33]:vKill;a410
19<- [+$OK#9A]

---------------------------------------------------
(gdb) set debug remote 1
(gdb) target remote :4321
Remote debugging using :4321
[remote] start_remote_1: enter
  [remote] Sending packet: $qSupported:multiprocess+;swbreak+;hwbreak+;qRelocInsn+;fork-events+;vfork-events+;exec-events+;vContSupported+;QThreadEvents+;QThreadOptions+;no-resumed+;memory-tagging+;xmlRegisters=i386#72
  [remote] Received Ack
  [remote] Packet received:
  [remote] packet_ok: Packet qSupported (supported-packets) is NOT supported
  [remote] Sending packet: $vCont?#49
  [remote] Received Ack
  [remote] Packet received:
  [remote] packet_ok: Packet vCont (verbose-resume) is NOT supported
  [remote] Sending packet: $vMustReplyEmpty#3a
  [remote] Received Ack
  [remote] Packet received:
  [remote] Sending packet: $Hg0#df
  [remote] Received Ack
  [remote] Packet received:
  [remote] Sending packet: $qTStatus#49
  [remote] Received Ack
  [remote] Packet received:
  [remote] packet_ok: Packet qTStatus (trace-status) is NOT supported
  [remote] Sending packet: $?#3f
  [remote] Received Ack
  [remote] Packet received: S05
  [remote] Sending packet: $qfThreadInfo#bb
  [remote] Received Ack
  [remote] Packet received:
  [remote] Sending packet: $qL1160000000000000000#55
  [remote] Received Ack
  [remote] Packet received:
  [remote] Sending packet: $Hc-1#09
  [remote] Received Ack
  [remote] Packet received:
  [remote] Sending packet: $qC#b4
  [remote] Received Ack
  [remote] Packet received:
  [remote] Sending packet: $qAttached#8f
  [remote] Received Ack
  [remote] Packet received:
  [remote] packet_ok: Packet qAttached (query-attached) is NOT supported
  [remote] Sending packet: $qOffsets#4b
  [remote] Received Ack
  [remote] Packet received:
  [remote] wait: enter
    [remote] select_thread_for_ambiguous_stop_reply: enter
      [remote] select_thread_for_ambiguous_stop_reply: process_wide_stop = 0
      [remote] select_thread_for_ambiguous_stop_reply: first resumed thread is Thread <main>
      [remote] select_thread_for_ambiguous_stop_reply: is this guess ambiguous? = 0
    [remote] select_thread_for_ambiguous_stop_reply: exit
  [remote] wait: exit
  [remote] Sending packet: $g#67
  [remote] Received Ack
  [remote] Packet received: 0000000000000000000000000000000000000000000000000000000000000000
  [remote] Sending packet: $p20#d2
  [remote] Received Ack
  [remote] Packet received: 00000000
  [remote] packet_ok: Packet p (fetch-register) is supported
  [remote] Sending packet: $qL1160000000000000000#55
  [remote] Received Ack
  [remote] Packet received:
  [remote] Sending packet: $m0,4#fd
  [remote] Received Ack
  [remote] Packet received: 00000000
  [remote] Sending packet: $mfffffffc,4#fa
  [remote] Received Ack
  [remote] Packet received: 00000000
0x00000000 in rvtest_init ()
  [remote] Sending packet: $qSymbol::#5b
  [remote] Received Ack
  [remote] Packet received:
  [remote] packet_ok: Packet qSymbol (symbol-lookup) is NOT supported
[remote] start_remote_1: exit
(gdb) k
Kill the program being debugged? (y or n) y
[remote] Sending packet: $vKill;a410#33
[remote] Received Ack
[remote] Packet received: OK
[remote] packet_ok: Packet vKill (kill) is supported
[Inferior 1 (Remote target) killed]


*/

// https://sourceware.org/gdb/current/onlinedocs/gdb.html/Packets.html#Packets
// https://ftp.gnu.org/old-gnu/Manuals/gdb/html_node/gdb_129.html
namespace gdb_remote
{
    enum Command: char
    {
        /// Indicate the reason the target halted. The reply is the same as for step and continue.
        LAST_SIGNAL = '?',
        CONTINUE_c = 'c', // continue
        CONTINUE_C = 'C', // continue with signal
        STEP_s = 's', // single step
        STEP_S = 'S', // step with signal
        DETACH = 'D',
        /// get all registers
        GP_REG_GET = 'g',
        /// set registers
        GP_REG_SET = 'G',
        KILL_TGT = 'k',
        /// read memory: m<addr>,<size>
        MEM_GET = 'm',
        /// write memory: m<addr>,<size>:data
        MEM_SET = 'M',
        /// read memory, binary
        MEM_BIN_GET = 'x',
        /// write memory, binary
        MEM_BIN_SET = 'X',
        /// get register pHH
        REG_GET = 'p',
        /// set register PHH=xx...
        REG_SET = 'P',
        /// generic query, get value
        GENERIC_Q_GET = 'q',
        /// generic query, set value
        GENERIC_Q_SET = 'Q',
        /// multi-letter query
        QUERY_V = 'v',
        /// remove breakpoint/watchpoint: z<type>,<addr>,<length>
        BREAK_CLR = 'z',
        /// insert breakpoint/watchpoint: z<type>,<addr>,<length>
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
        /// data start mark
        GDB_BEG = '$',
        /// data end mark
        GDB_END = '#',
        /// next char is escaped and XOR-ed with 0x20
        GDB_ESC = '}',
        /// encode/decode escaped char: c = c ^ 0x20
        GDB_XOR = 0x20, // just ' ' - space
        /// RLE encoded, next char - length
        GDB_RLE_MARK = '*',
        /// rle_length = c - 28
        GDB_RLE_LENGTH = 28,
        /// ctr+C - user request interrupt
        GDB_BREAK = 0x03,
    };
} // namespace gdb_remote

int main(int argc, char ** argv)
{
    asio::io_context ctx;

    std::array<uint32_t, 33> regs{};
    std::vector<uint8_t> ram;
    ram.resize(0x800'0000, 0);

    tcp::acceptor server(ctx, tcp::endpoint(tcp::v4(), 4321));

    tcp::socket client(ctx);
    std::cout << "wait for connection" << std::endl;
    server.accept(client);
    std::cout << "new connection" << std::endl;
    try
    {
        tcp::iostream stream;
        bool run = true;
        auto make_answer = [](const std::string& data)
        {
            uint8_t sum = 0;
            for (uint8_t c: data)
                sum += c;
            return std::format("${}#{:02X}", data, sum);
        };

        auto make_ack = [&make_answer](const std::string& data, bool with_mark = true)
        {
            using namespace gdb_remote;
            if (with_mark) return char(GDB_ACK) + make_answer(data);
            return make_answer(data);
        };

        auto make_nack = [&make_answer](const std::string& data = "", bool with_mark = true)
        {
            using namespace gdb_remote;
            if (with_mark) return char(GDB_NAK) + make_answer(data);
            return make_answer(data);
        };

        auto encode_rle = [](char c, char cnt)
        {
            return std::format("{}*{}", c, char(cnt + 28));
        };

        auto encode_reg = [](uint32_t r)
        {
            auto v = reinterpret_cast<const uint8_t*>(&r);
            return std::format("{:02X}{:02X}{:02X}{:02X}", v[0], v[1], v[2], v[3]);
        };

        int state = 0;
        do
        {
            ++state;
            using namespace gdb_remote;
            std::string input, output, args;

            asio::read_until(client, asio::dynamic_buffer(input), "#");
            auto b_pos = input.find(Protocol::GDB_BEG);
            auto cmd = input.substr(b_pos);
            auto e_pos = cmd.find(Protocol::GDB_END);
            cmd = cmd.substr(1, e_pos - 1);
            args = cmd.substr(1);
            std::cout << state << "-> [" << input << "]:" << cmd << std::endl;
            // TODO: calc checksum
            if (cmd.empty())
                continue;
            switch (cmd[0])
            {
                case GENERIC_Q_GET:
                {
                    std::cout << "GENERIC_Q_GET" << std::endl;
                    if (cmd.starts_with("qSupported:")) // qSupported (supported-packets)
                        output = make_ack("PacketSize=2048;hwbreak+;swbreak+"); // 'PacketSize' is required
                    else if (cmd == "qOffsets")
                        output = make_ack("Text=0;Data=400000;Bss=800000"); // Note: hex(?) values
//                    else if (cmd.starts_with("qTStatus"))
//                        output = make_ack("");
//                    else if (cmd.starts_with("qSymbol:"))
//                        output = make_ack("OK");
                    else if (cmd.starts_with("qfThreadInfo")) // threads info / replacement for 'qL'
                        output = make_ack("l"); // no threads
//                    else if (cmd == "qAttached")
//                        output = make_ack("");
                    else if (cmd == "qC")
                        output = make_ack("-1");
                    else
                        output = make_ack(""); // should return "$#00' if command is unknown
                    break;
                }
                case QUERY_V: // not supported
                {
                    std::cout << "QUERY_V" << std::endl;
//                    if (cmd == "vCont?") // vCont (verbose-resume)
//                        output = make_ack("vCont:no"); // The stub must support ‘vCont’ if it reports support for multiprocess extensions
//                    else if (cmd.starts_with("vCtrlC"))
//                        output = make_ack("OK"); // NOLINT(bugprone-branch-clone)
//                    else if (cmd.starts_with("vKill"))
//                        output = make_ack("OK");
//                    else
                        output = make_ack("");
                    break;
                }
                case LAST_SIGNAL:
                {
                    std::cout << "LAST_SIGNAL" << std::endl;
                    output = make_ack(std::format("S{:02X}", SIGTRAP));
                    break;
                }
                case THREAD_SET:
                {
                    std::cout << "THREAD_SET" << std::endl;
                    output = make_ack("OK");
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
                    output = make_ack(rx);
                    break;
                }
                case GP_REG_SET: // set GP registers
                {
                    std::cout << "GP_REG_SET" << std::endl;
                    output = make_ack("E01");
                    break;
                }
                case REG_GET: // pHH - get register 0xHH
                {
                    auto rid = std::stoul(args, nullptr, 16);
                    std::cout << "REG_GET: " << rid << std::endl;
                    output = make_ack(encode_reg(regs[rid]));
                    break;
                }
                case REG_SET: // pHH=value - set register 0xHH
                {
                    std::cout << "REG_SET: "
                        << std::stoul(args, nullptr, 16) << std::endl;
                    output = make_ack("E02");
                    break;
                }
                case MEM_GET: // mADR,SZ - read memory
                {
                    size_t idx_addr = 0;
                    size_t idx_size = 0;
                    size_t addr = std::stoul(args, &idx_addr, 16);
                    size_t size = std::stoul(args.substr(idx_addr + 1), &idx_size, 16);
                    std::cout << "MEM_GET: " << std::hex << addr << ":" << std::dec << size << std::endl;
                    if ((addr + size) >= ram.size())
                    {
                        output = make_ack("");
                    }
                    else
                    {
                        std::string res;
                        res.reserve(size * 2);
                        for (auto b: std::span(ram.data() + addr, size))
                        {
                            res += std::format("{:02X}", b);
                        }
                        output = make_ack(res);
                    }
                    break;
                }
                case MEM_SET: // mADR,SZ:data - write memory
                {
                    size_t idx_addr = 0;
                    size_t idx_size = 0;
                    uint32_t addr = std::stoul(args, &idx_addr, 16);
                    uint32_t size = std::stoul(args.substr(idx_addr + 1), &idx_size, 16);
                    std::cout << "MEM_SET: " << std::hex << addr << ":" << std::dec << size << std::endl;
                    if ((addr + size) >= ram.size())
                    {
                        output = make_ack("");
                    }
                    else
                    {
                        output = make_ack("OK");
                    }
                    break;
                }
                case BREAK_CLR:
                case BREAK_SET:
                {
                    std::cout << "BREAKPOINT: " << args << std::endl;
                    output = make_ack("OK");
                    break;
                }
                case STEP_s:
                case STEP_S:
                {
                    std::cout << "STEP: " << args << std::endl;
                    output = make_ack("S03"); // SIGQUIT
                    break;
                }
                case CONTINUE_C:
                {
                    std::cout << "CONTINUE: " << args << std::endl;
                    output = make_ack("S06"); // SIGABRT
                    break;
                }
                case CONTINUE_c: // exec until next stop
                {
                    std::cout << "CONTINUE" << std::endl;
                    // stop cause:
                    // SAA - signal AA received
                    // WAA - exit with code AA
                    // XAA - terminated with AA signal
                    regs.back() += 4; // simulate PC increment
                    output = make_ack("S05"); // S05 == SIGTRAP
                    break;
                }
                case DETACH: // debugger detached, exit
                    std::cout << "DETACH" << std::endl;
                    output = make_ack("OK");
                    run = false;
                    break;
                case KILL_TGT: // stop execution, exit
                    std::cout << "KILL_TGT" << std::endl;
                    output = make_ack("OK");
                    run = false;
                    break;
                default: // "unknown command"
                    std::cout << "unknown command, ignored" << std::endl;
                    output = make_ack("");
                    break;
            }
            std::cout << state << "<- [" << output << "]" << std::endl;
            asio::write(client, asio::buffer(output));
        } while (run);
    }
    catch (...)
    {}
    return 0;
}
