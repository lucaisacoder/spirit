#include <nanogui/tabheader.h>
#include <nanogui/glcanvas.h>
#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/colorpicker.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>

#include <iostream>
#include <string>
#include <vector>
#include <memory>

// Includes for the GLTexture class.
#include <cstdint>
#include <utility>

#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#if defined(_WIN32)
#  pragma warning(push)
#  pragma warning(disable: 4457 4456 4005 4312)
#endif

#if defined(_WIN32)
#  pragma warning(pop)
#endif
#if defined(_WIN32)
#  if defined(APIENTRY)
#    undef APIENTRY
#  endif
#  include <windows.h>
#endif

#include <GLCanvas.hpp>
#include <ConfigurationsWindow.hpp>
#include <EnergyGraph.hpp>
#include <MethodWidget.hpp>
#include <MainWindowLayout.hpp>

#include <VisualisationWidget.hpp>
#include <Spirit/State.h>
#include <Spirit/System.h>
#include <Spirit/Chain.h>
#include <Spirit/Hamiltonian.h>
#include <Spirit/Configurations.h>
#include <Spirit/Transitions.h>
#include <Spirit/Simulation.h>
#include <Spirit/Version.h>
#include <Spirit/Log.h>

#ifdef _OPENMP
    #include <omp.h>
#endif

#ifdef __APPLE__
auto CTRL = GLFW_MOD_SUPER;
#else
auto CTRL = GLFW_MOD_CONTROL;
#endif

class MainWindow : public nanogui::Screen
{
public:
    MainWindow(int width, int height, std::shared_ptr<State> state)
        : nanogui::Screen(Eigen::Vector2i{width, height}, std::string("NanoGUI - Spirit v")+Spirit_Version(), true),
        state(state)
    {
        using namespace nanogui;
        using Anchor = AdvancedGridLayout::Anchor;
        this->setSize({800,600});
        
        auto grid = new MainWindowLayout({200,200}, {200, 30});

        this->setLayout(grid);
        grid->setRowStretch(0,1);
        grid->setRowStretch(1,0);
        grid->setColStretch(0,1);
        grid->setColStretch(1,0);

        this->gl_canvas = new VFGLCanvas(this, state);
        gl_canvas->setSize({400,400});
        grid->setAnchor(gl_canvas, Anchor(0,0,1,1, Alignment::Fill, Alignment::Fill));

        auto w = new Window(this, "");
        w->setLayout(new GroupLayout());
        w->setSize({300,230});
        w->setPosition({0,0});
        this->energy_graph = new EnergyGraph(w, state);
        this->energy_graph->setSize({300,200});
        this->energy_graph->updateData();
        // grid->setAnchor(w, Anchor(0,0,1,1, Alignment::Middle, Alignment::Middle));

        auto vis = new VisualisationWidget(this, gl_canvas);
        vis->setPosition({0,0});
        vis->setSize({200,300});

        // grid->setAnchor(vis, Anchor(0,0,1,1, Alignment::Middle, Alignment::Middle));
        vis->addRenderer(new ArrowRendererWidget(vis, gl_canvas));

        this->configurations = new ConfigurationsWindow(this, state);
        grid->setAnchor(configurations, Anchor(1,0,1,2,Alignment::Fill, Alignment::Fill));
        configurations->setLayout(new GroupLayout());
        configurations->setUpdateCallback([&] {
            System_Update_Data(state.get());
            Chain_Update_Data(state.get());
            this->gl_canvas->updateData();
            this->energy_graph->updateData();
        });

        this->method = new MethodWidget(this, state);
        grid->setAnchor(method, Anchor(0,1,1,1, Alignment::Fill, Alignment::Fill));

        performLayout();
    }

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers)
    {
        if( Screen::keyboardEvent(key, scancode, action, modifiers) )
            return true;
        if( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
        {
            setVisible(false);
            return true;
        }
        if( key == GLFW_KEY_SPACE && action == GLFW_PRESS && !modifiers )
        {
            this->method->start_stop();
            this->method->update();
        }
        if( key == GLFW_KEY_LEFT && action == GLFW_PRESS && !modifiers )
        {
            if( System_Get_Index(state.get()) > 0 )
            {
                Chain_prev_Image(state.get());
                this->gl_canvas->updateData();
                this->energy_graph->updateData();
            }
            this->method->update();
        }
        if( key == GLFW_KEY_RIGHT && action == GLFW_PRESS && !modifiers )
        {
            if( System_Get_Index(state.get()) < Chain_Get_NOI(state.get())-1 )
            {
                Chain_next_Image(state.get());
                this->gl_canvas->updateData();
                this->energy_graph->updateData();
            }
            this->method->update();
        }
        if( key == GLFW_KEY_X && action == GLFW_PRESS && modifiers == CTRL )
        {
            Chain_Image_to_Clipboard(state.get());
            Chain_Delete_Image(state.get());
            this->method->update();
        }
        if( key == GLFW_KEY_V && action == GLFW_PRESS && modifiers == CTRL )
        {
            Chain_Replace_Image(state.get());
            Chain_Update_Data(state.get());
            this->energy_graph->updateData();
        }
        if( key == GLFW_KEY_C && action == GLFW_PRESS && modifiers == CTRL )
        {
            Chain_Image_to_Clipboard(state.get());
        }
        if( key == GLFW_KEY_LEFT && action == GLFW_PRESS && modifiers == CTRL )
        {
            Chain_Insert_Image_Before(state.get());
            Chain_Update_Data(state.get());
            this->energy_graph->updateData();
            this->method->update();
            this->method->updateThreads();
        }
        if( key == GLFW_KEY_RIGHT && action == GLFW_PRESS && modifiers == CTRL )
        {
            Chain_Insert_Image_After(state.get());
            Chain_Update_Data(state.get());
            this->energy_graph->updateData();
            this->method->update();
            this->method->updateThreads();
        }
        if( key == GLFW_KEY_R && action == GLFW_PRESS && modifiers == CTRL )
        {
            Configuration_Random(state.get());
            Chain_Update_Data(state.get());
            this->energy_graph->updateData();
            this->gl_canvas->updateData();
        }
        return false;
    }

    virtual bool resizeEvent(const Eigen::Vector2i & size)
    {
        nanogui::Screen::resizeEvent(size);
        // this->gl_canvas->setSize(size);
        this->gl_canvas->drawGL();
        performLayout();
        return true;
    }

    virtual void draw(NVGcontext *ctx)
    {
        Screen::draw(ctx);
    }

    virtual void drawContents()
    {
        Screen::drawContents();
        this->gl_canvas->drawGL();
    }

    void performLayout()
    {
        std::cout << "mwindow layout \n";
        Screen::performLayout();
        // A bit of a hack, we need to do this since Widget::setSize is non-virtual, 
        // and the layout container therefore does not adjust the framebuffer size properly
        this->gl_canvas->setSize(this->gl_canvas->size());
    }

private:
    std::shared_ptr<State> state;
    VFGLCanvas *gl_canvas;
    ConfigurationsWindow * configurations;
    EnergyGraph *energy_graph;
    MethodWidget *method;
    nanogui::TabHeader *header;
};

int main(int /* argc */, char ** /* argv */)
try
{
    std::string cfgfile = "input/input.cfg";
    bool quiet = false;
    auto state = std::shared_ptr<State>(State_Setup(cfgfile.c_str(), quiet), State_Delete);
    Configuration_PlusZ(state.get());
    Configuration_Skyrmion(state.get(), 5, 1, -90, false, false, false);

    #ifdef _OPENMP
        int nt = omp_get_max_threads() - 1;
        Log_Send(state.get(), Log_Level_Info, Log_Sender_UI, ("Using OpenMP with n=" + std::to_string(nt) + " threads").c_str());
    #endif

    nanogui::init();

    /* scoped variables */
    {
        nanogui::ref<MainWindow> app = new MainWindow(800, 600, state);
        app->drawAll();
        app->setVisible(true);
        nanogui::mainloop();
    }

    nanogui::shutdown();
}
catch (const std::runtime_error &e)
{
    std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
    #if defined(_WIN32)
        MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
    #else
        std::cerr << error_msg << std::endl;
    #endif
    return -1;
}