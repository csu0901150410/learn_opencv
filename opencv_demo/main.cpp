
#include "sciter-x.h"
#include "sciter-x-graphics.hpp"
#include "sciter-x-window.hpp"
#include "sciter-om.h"

#include <functional>

// native API demo

class frame : public sciter::window
{
public:
    frame() : window(SW_TITLEBAR | SW_RESIZEABLE | SW_CONTROLS /*| SW_TOOL*/ | SW_MAIN | SW_ENABLE_DEBUG) {}

    SOM_PASSPORT_BEGIN(frame)

    SOM_FUNCS(SOM_FUNC(helloWorld))

    SOM_PASSPORT_END

    // function for script: view.frame.helloWorld()
    sciter::string helloWorld()
    {
        return WSTR("Hello TISript!");
    }
};

#include "resources.cpp"

int uimain(std::function<int()> run)
{
    sciter::archive::instance().open(aux::elements_of(resources)); // bind resources[] (defined in "resources.cpp") with the archive

    sciter::om::hasset<frame> pwin = new frame();

    // note: this:://app URL is dedicated to the sciter::archive content associated with the application
    pwin->load(WSTR("this://app/main.htm"));

    pwin->expand();

    return run();
}
