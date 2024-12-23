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
        GP_REG_GET = 'g',
        GP_REG_SET = 'G',
        KILL_TGT = 'k',
        /// read memory: m<addr>,<size>
        MEM_GET = 'm',
        MEM_SET = 'M',
        REG_GET = 'p',
        REG_SET = 'P',
        GENERIC_Q_GET = 'q',
        GENERIC_Q_SET = 'Q',
        /// multi-letter query
        QUERY_V = 'v',
        /// write memory, binary
        LOAD_BIN = 'X',
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
        /// rle_length = c - 0x28
        GDB_RLE_LENGTH = 0x28, // '('
        /// ctr+C - user request interrupt
        GDB_BREAK = 0x03,
    };
} // namespace gdb_remote

int main(int argc, char ** argv)
{
    asio::io_context ctx;

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

        int state = 0;
        do
        {
            ++state;
            using namespace gdb_remote;
            std::string input, output;

            asio::read_until(client, asio::dynamic_buffer(input), "#");
            auto b_pos = input.find(Protocol::GDB_BEG);
            auto cmd = input.substr(b_pos);
            auto e_pos = cmd.find(Protocol::GDB_END);
            cmd = cmd.substr(1, e_pos - 1);
            std::cout << state << "-> [" << input << "]:" << cmd << std::endl;
            // TODO: calc checksum
            if (cmd.empty())
                continue;
            switch (cmd[0])
            {
                case GENERIC_Q_GET:
                {
                    if (cmd.starts_with("qSupported"))
                        output = make_ack(""); // NOLINT(bugprone-branch-clone)
//                    else if (cmd.starts_with("qTStatus"))
//                        output = make_ack("");
//                    else if (cmd.starts_with("qOffsets"))
//                        output = make_ack("");
//                    else if (cmd.starts_with("qSymbol:"))
//                        output = make_ack("OK");
//                    else if (cmd.starts_with("qfThreadInfo")) // threads info / replacement for 'qL'
//                        output = make_ack("l"); // no threads
//                    else if (cmd == "qAttached")
//                        output = make_ack("");
//                    else if (cmd == "qC")
//                        output = make_ack("");
                    else
                        output = make_ack(""); // should return "$#00' if command is unknown
                    break;
                }
                case QUERY_V:
                {
                    if (cmd.starts_with("vCont?"))
                        output = make_ack(""); // NOLINT(bugprone-branch-clone)
                    else if (cmd.starts_with("vKill"))
                        output = make_ack("OK");
                    else
                        output = make_ack("");
                    break;
                }
                case LAST_SIGNAL:
                {
                    output = make_ack("S05");
                    break;
                }
                case THREAD_SET:
                {
                    output = make_ack("");
                    break;
                }
                case GP_REG_GET: // get all GP registers
                {
                    output = make_ack(std::string(2*32, '0'));
                    break;
                }
                case GP_REG_SET: // set GP registers
                {
                    output = make_ack("");
                    break;
                }
                case REG_GET: // pHH - get register 0xHH
                {
                    output = make_ack("00000000");
                    break;
                }
                case REG_SET: // pHH=value - set register 0xHH
                {
                    output = make_ack("");
                    break;
                }
                case MEM_GET: // mADR,SZ - read memory
                {
                    output = make_ack("00000000");
                    break;
                }
                case MEM_SET: // mADR,SZ:data - write memory
                {
                    output = make_ack("");
                    break;
                }
                case CONTINUE_c: // exec until next stop
                {
                    output = make_ack("OK");
                    break;
                }
                case DETACH: // debugger detached, exit
                    output = make_ack("OK");
                    run = false;
                    break;
                default: // "unknown command"
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
