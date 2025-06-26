#include <wx/init.h>
#include <iostream>

int main()
{
    // Initialize wxWidgets manually without wxIMPLEMENT_APP
    if (!wxInitialize()) {
        return 1;
    }
        
    // Clean up
    wxUninitialize();
    return 0;
}
