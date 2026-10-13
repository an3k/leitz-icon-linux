#include "original_status_decoder.h"
#include <string.h>

/* Strings below are transcribed from the original 5.1.465 Mach-O's
 * _CheckPrinterStatus disassembly. The two substring tests for empty roll
 * are combined by the OEM at 0x100009668..0x10000969c.
 *
 * The OEM unconditionally selects 'ready' at 0x1000098ff..0x100009972
 * for unrecognized, nonempty text. The intervening incompatible-cartridge
 * message block is not reached through that active default branch.
 */
classic_original_status_decision
classic_original_decide_status(const char *reply,long received) {
    classic_original_status_decision d={0};
    if(received < 0) {
        d.state="com.leitz.general-error";
        d.info="General error occurred, will retry in 5 seconds.";
    } else if(received == 0) {
        d.state="com.leitz.general-error";
        d.info="General error occurred, will retry in 5 seconds.";
    } else if(strstr(reply,"SKU:Load Error;") || strstr(reply,"USED:-1")) {
        d.state="com.leitz.out-of-paper-error";
        d.info="Out of labels, will retry in 5 seconds.";
        d.send_reset_and_end=1;
        d.sleep_seconds=5;
    } else if(strstr(reply,"SKU:Door Open;")) {
        d.state="com.leitz.door-open-error";
        d.info="The door is open, will retry in 5 seconds.";
        d.sleep_seconds=5;
    } else if(strstr(reply,"SKU:No Cartridge;")) {
        d.state="com.leitz.no-cartridge-error";
        d.info="There is no cartridge in the printer, will retry in 5 seconds.";
        d.sleep_seconds=5;
    } else if(strstr(reply,"SKU:Bad Cartridge;")) {
        d.state="com.leitz.general-error";
        d.info="General error occurred, will retry in 5 seconds.";
    } else {
        d.state="com.leitz.ready";
        d.info="The printer is ready.";
    }
    return d;
}
