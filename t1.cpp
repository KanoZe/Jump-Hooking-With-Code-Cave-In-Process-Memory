#include <iostream>
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <vector>
using namespace std;
int findMyProc(const char *procname) {
    HANDLE hSnapshot;
    PROCESSENTRY32 pe;
    int pid = 0;
    BOOL hResult;

    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hSnapshot) return 0;

    pe.dwSize = sizeof(PROCESSENTRY32);
    hResult = Process32First(hSnapshot, &pe);

    while (hResult) {
        if (strcmp(procname, pe.szExeFile) == 0) {
            pid = pe.th32ProcessID;
            break;
        }
        hResult = Process32Next(hSnapshot, &pe);
    }

    CloseHandle(hSnapshot);
    return pid;
}


int main() {

    int pid = findMyProc("wesnoth.exe");
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    
    HMODULE hMods[1024];
    DWORD cbNeeded;

    if(EnumProcessModulesEx(hProcess,hMods,sizeof(hMods),&cbNeeded,LIST_MODULES_ALL)){
        cout << "Address of main module is " <<  hMods[0] << endl;
    }

    // uint64_t mainOffset = 0x01736D20;
    uint64_t mainOffset = 0x017370A8;
    uint64_t addr = ((uint64_t)hMods[0]) + mainOffset;

    std::vector<unsigned int> offsets = { 0x0, 0x488, 0x10, 0x78, 0x8, 0x8 };


    for(int i =0; i < offsets.size(); i++){
        ReadProcessMemory(hProcess,(BYTE*)addr,&addr,sizeof(addr),nullptr);
        addr += offsets[i];
    }


    uint64_t value = 0;
    ReadProcessMemory(hProcess, (BYTE*)addr, &value, sizeof(uint32_t), nullptr);
        
    uint64_t jmpRaxOffset = (uint64_t)hMods[0] + 0xA428AE + 0x1000;
    uint64_t codeCaveOffset =(uint64_t)hMods[0] + 0x11DAE5E + 0x1000;
    uint64_t jmpBack =(uint64_t)hMods[0] + 0xA428B3 + 0x1000;
    cout << "code cave offset is " << codeCaveOffset << endl;
    cout << "code cave offset is hex " << hex << codeCaveOffset << endl;
        
    // The machine code for 'ret' (which is 'C3')
    uint64_t codeCaveRelativeOffset = codeCaveOffset - (jmpRaxOffset + 5);
    cout << "code cave RELATIVE offset is " << codeCaveRelativeOffset << endl;
    cout << "code cave RELATIVE offset is hex " << hex << codeCaveRelativeOffset << endl;

    BYTE codeAtInitialJmp[5];
    codeAtInitialJmp[0] = 0xE9;
    memcpy(&codeAtInitialJmp[1],&codeCaveRelativeOffset,sizeof(codeCaveRelativeOffset));
    
    // Print out the bytes for verification
    for (int i = 0; i < 5; i++) {
        printf("%02X ", codeAtInitialJmp[i]);
    }
    printf("\n");




    BYTE codeCave[21] = { 
    0x48, 0xBB, /*(BYTE)addr*/ 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
    0xC7, 0x03, 0xE7, 0x03, 0x00,0x00, 
    0xE9, /*jmpBack*/ 0x00,0x00,0x00,0x00};


    uint64_t jmpRaxRelativeOffset = jmpRaxOffset - (codeCaveOffset + 11);
    cout << "JMP RAX RELATIVE offset is " << jmpRaxRelativeOffset << endl;
    cout << "JMP RAX RELATIVE offset is hex " << hex << jmpRaxRelativeOffset << endl;
    memcpy(&codeCave[2],&addr,sizeof(addr));
    memcpy(&codeCave[17],&jmpRaxRelativeOffset,sizeof(jmpRaxRelativeOffset));
    //should overwrite jmp rax
   if(WriteProcessMemory(hProcess,(BYTE*)jmpRaxOffset ,codeAtInitialJmp,sizeof(codeAtInitialJmp), 0)){
    cout << "Wrote the jmp rax" << codeAtInitialJmp << endl;    
        // Print out the bytes for verification
    for (int i = 0; i < 5; i++) {
        printf("%02X ", codeAtInitialJmp[i]);
    }
    printf("\n");


   }else{
        cout << "Failed to write jmp" << endl;
   }

    //should write code cave
    if(WriteProcessMemory(hProcess,(BYTE*)codeCaveOffset,codeCave,sizeof(codeCave), 0)){
    cout << "Wrote the code cave" << endl;    
   }else{
        cout << "Failed to write cave" << endl;
   }
    CloseHandle(hProcess);
    while(true){
      Sleep(10000);
    }
    return 0;
}
