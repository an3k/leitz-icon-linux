#include "wifi_xml.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define SCAN "<response><ScanInProgress>0</ScanInProgress><NumScanResults>2</NumScanResults><MySsid>Home &amp; Office</MySsid></response>"
#define ROW "<response><bss><row><Valid>1</Valid><Name>Net &amp; Lab</Name><SecurityMode> 13 </SecurityMode><NetworkType>2</NetworkType><Strength>-37</Strength></row></bss></response>"
int main(void)
{
    leitz_win2018_scan_xml scan={0};
    assert(leitz_win2018_parse_scan_xml(SCAN,strlen(SCAN),&scan)==0);
    assert(!scan.in_progress && scan.number_of_results==2);
    assert(!strcmp(scan.my_ssid,"Home & Office"));
    const char *in_progress="<response><ScanInProgress>1</ScanInProgress></response>";
    assert(leitz_win2018_parse_scan_xml(in_progress,strlen(in_progress),&scan)==0);
    assert(scan.in_progress && !scan.number_of_results && !scan.my_ssid[0]);
    const char *ordinal="<response><ScanInProgress> 1 </ScanInProgress><NumScanResults>0</NumScanResults><MySsid/></response>";
    assert(leitz_win2018_parse_scan_xml(ordinal,strlen(ordinal),&scan)==0);
    assert(!scan.in_progress);

    leitz_win2018_bss_xml row={0};
    assert(leitz_win2018_parse_bss_xml(ROW,strlen(ROW),&row)==0);
    assert(row.valid && !strcmp(row.name,"Net & Lab"));
    assert(row.security_mode==13 && row.network_type==2 && row.strength==-37);
    const char*invalid="<response><bss><r><Valid>0</Valid></r></bss></response>";
    assert(leitz_win2018_parse_bss_xml(invalid,strlen(invalid),&row)==0);
    assert(!row.valid && !row.name[0]);
    leitz_win2018_bss_xml snapshot=row;
    const char*bad="<response><bss><r><Valid>1</Valid><Name>Foo</Name><SecurityMode>13</SecurityMode><NetworkType>2</NetworkType><Strength>bad</Strength></r></bss></response>";
    assert(leitz_win2018_parse_bss_xml(bad,strlen(bad),&row)==-1);
    assert(!memcmp(&snapshot,&row,sizeof row));
    const char*missing="<response><bss><r><Name>X</Name></r></bss></response>";
    assert(leitz_win2018_parse_bss_xml(missing,strlen(missing),&row)==-1);
    const char*injected="<!DOCTYPE response [<!ENTITY x 'danger'>]><response><ScanInProgress>&x;</ScanInProgress></response>";
    assert(leitz_win2018_parse_scan_xml(injected,strlen(injected),&scan)==-1);
    const char*duplicated="<response><ScanInProgress>1</ScanInProgress><ScanInProgress>0</ScanInProgress></response>";
    assert(leitz_win2018_parse_scan_xml(duplicated,strlen(duplicated),&scan)==-1);
    const char*truncated="<response><ScanInProgress>1</ScanInProgress>";
    assert(leitz_win2018_parse_scan_xml(truncated,strlen(truncated),&scan)==-1);
    const char*overflow="<response><ScanInProgress>0</ScanInProgress><NumScanResults>2147483648</NumScanResults><MySsid>X</MySsid></response>";
    assert(leitz_win2018_parse_scan_xml(overflow,strlen(overflow),&scan)==-1);
    const char*negative="<response><ScanInProgress>0</ScanInProgress><NumScanResults>-1</NumScanResults><MySsid>X</MySsid></response>";
    assert(leitz_win2018_parse_scan_xml(negative,strlen(negative),&scan)==-1);
    const char*unknown="<response><ScanInProgress>1</ScanInProgress><U>unrelated</U></response>";
    assert(leitz_win2018_parse_scan_xml(unknown,strlen(unknown),&scan)==0);
    assert(leitz_win2018_parse_scan_xml(NULL,20,&scan)==-1);
    assert(leitz_win2018_parse_scan_xml(SCAN,strlen(SCAN),NULL)==-1);
    assert(leitz_win2018_parse_bss_xml(ROW,strlen(ROW),NULL)==-1);
    puts("Windows AddPrinter 2018 bounded WiFi scan and BSS XML: PASS");
    return 0;
}
