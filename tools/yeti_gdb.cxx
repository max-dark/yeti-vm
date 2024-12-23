#include <asio.hpp>

#include <iostream>
#include <format>
#include <string>
#include <vector>

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
        GP_REG_GET = 'g',
        GP_REG_SET = 'G',
        KILL_TGT = 'k',
        MEM_GET = 'm',
        MEM_SET = 'M',
        REG_GET = 'p',
        REG_SET = 'P',
        GENERIC_Q_GET = 'q',
        GENERIC_Q_SET = 'Q',
        QUERY_V = 'v',
        LOAD_BIN = 'X',
        BREAK_CLR = 'z',
        BREAK_SET = 'Z',
        /// H<cmd><thread> set current thread for command
        THREAD_SET = 'H',
    };

    enum Protocol: char
    {
        /// command accepted
        GDB_ACK = '+',
        /// command rejected
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
        /// rle_length = c - 0x28
        GDB_RLE_LENGTH = 0x28,
    };
} // namespace gdb_remote

int main(int argc, char ** argv)
{
    asio::io_context ctx;

    tcp::acceptor server(ctx, tcp::endpoint(tcp::v4(), 4321));

    tcp::socket client(ctx);
    server.accept(client);
    std::cout << "wait for connection" << std::endl;
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

        int state = 0;
        do
        {
            ++state;
            using namespace gdb_remote;
            std::string input, output;

            asio::read_until(client, asio::dynamic_buffer(input), "#");
            //client.read_some(asio::buffer(input));
            std::cout << state << ':' << input << std::endl;
            auto b_pos = input.find(Protocol::GDB_BEG);
            auto cmd = input.substr(b_pos);
            auto e_pos = cmd.find(Protocol::GDB_END);
            cmd = cmd.substr(1, e_pos - 1);
            std::cout << state << '>' << cmd << std::endl;
            // TODO: calc checksum
            if (cmd.empty())
                continue;
            switch (cmd[0])
            {
                case GENERIC_Q_GET:
                {
                    if (cmd.starts_with("qSupported"))
                        output = char(GDB_ACK) + make_answer(""); // NOLINT(bugprone-branch-clone)
                    else if (cmd.starts_with("qTStatus"))
                        output = char(GDB_ACK) + make_answer("");
                    else if (cmd.starts_with("qOffsets"))
                        output = char(GDB_ACK) + make_answer("");
                    else if (cmd.starts_with("qSymbol::"))
                        output = char(GDB_ACK) + make_answer("");
                    else if (cmd.starts_with("qfThreadInfo")) // threads info / replacement for 'qL'
                        output = char(GDB_ACK) + make_answer("l"); // no threads
                    else if (cmd.starts_with("qTStatus"))
                        output = char(GDB_ACK) + make_answer("");
                    else if (cmd == "qAttached")
                        output = char(GDB_ACK) + make_answer("");
                    else if (cmd == "qC")
                        output = char(GDB_ACK) + make_answer("");
                    else
                        output = char(GDB_NAK) + make_answer("E01");
                    break;
                }
                case QUERY_V:
                {
                    if (cmd.starts_with("vCont?"))
                        output = char(GDB_ACK) + make_answer(""); // NOLINT(bugprone-branch-clone)
                    else if (cmd.starts_with("vMustReplyEmpty"))
                        output = char(GDB_ACK) + make_answer("");
                    else if (cmd.starts_with("vKill"))
                        output = char(GDB_ACK) + make_answer("OK");
                    else
                        output = char(GDB_NAK) + make_answer("E01");
                    break;
                }
                case LAST_SIGNAL:
                {
                    output = char(GDB_ACK) + make_answer("S05");
                    break;
                }
                case THREAD_SET:
                {
                    output = char(GDB_ACK) + make_answer("OK");
                    break;
                }
                case GP_REG_GET: // get all GP registers
                {
                    output = char(GDB_ACK) + make_answer(std::string(2*32, '0'));
                    break;
                }
                case REG_GET: // pHH - get register 0xHH
                {
                    output = char(GDB_ACK) + make_answer("00000000");
                    break;
                }
                case MEM_GET: // mADR,SZ - read memory
                {
                    output = char(GDB_ACK) + make_answer("00000000");
                    break;
                }
                case CONTINUE_c: // exec until next stop
                {
                    output = char(GDB_ACK) + make_answer("OK");
                    break;
                }
                default:
                    output = char(GDB_NAK) + make_answer("");
                    break;
            }
            asio::write(client, asio::buffer(output));
        } while (run);
    }
    catch (...)
    {}
    return 0;
}
