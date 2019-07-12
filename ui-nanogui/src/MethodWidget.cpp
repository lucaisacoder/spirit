#include <MethodWidget.hpp>
#include <nanogui/button.h>
#include <nanogui/combobox.h>
#include <nanogui/layout.h>
#include <nanogui/entypo.h>
#include <nanogui/label.h>
#include "Spirit/Chain.h"
#include "Spirit/Log.h"
#include "Spirit/System.h"
#include <fmt/format.h>

MethodWidget::MethodWidget(nanogui::Widget * parent, std::shared_ptr<State> state)
: nanogui::Window(parent, ""), state(state)
{
    threads_image = std::vector<std::thread>(Chain_Get_NOI(this->state.get()));

    this->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Horizontal, nanogui::Alignment::Middle,0,10));
    start_stop_button  = new nanogui::Button(this, "Start", ENTYPO_ICON_CONTROLLER_PLAY);

    start_stop_button->setCallback([&] 
    {
        this->start_stop();
    });

    this->image_label = new nanogui::Label(this, fmt::format("{}/{}", System_Get_Index(this->state.get())+1, Chain_Get_NOI(this->state.get())));

    new nanogui::Label(this, "  Method: ");
    this->method_select = new ListSelect(this, method_strings);

    new nanogui::Label(this, "  Solver:");
    this->solver_select = new ListSelect(this, solver_strings);
}

MethodWidget::~MethodWidget()
{
    Simulation_Stop_All(state.get());

    for( unsigned int i=0; i<threads_image.size(); ++i )
    {
        if( threads_image[i].joinable() )
            threads_image[i].join();
    }
    if( thread_chain.joinable() )
        thread_chain.join();
}

void MethodWidget::update()
{
    this->image_label->setCaption(fmt::format(
        "{}/{}", System_Get_Index(this->state.get())+1, Chain_Get_NOI(this->state.get())) );
}

void MethodWidget::updateThreads()
{
    // Update thread arrays
    if( Chain_Get_NOI(state.get()) > (int)threads_image.size() )
    {
        for( int i=threads_image.size(); i < Chain_Get_NOI(state.get()); ++i )
            this->threads_image.push_back(std::thread());
    }
}

void MethodWidget::start_stop()
{
    Log_Send(state.get(), Log_Level_Debug, Log_Sender_UI, "Button: Start/Stop");
    Chain_Update_Data(this->state.get());
    int idx = System_Get_Index(this->state.get());

    if( Simulation_Running_On_Image(this->state.get()) ||
        Simulation_Running_On_Chain(this->state.get()) )
    {
        Simulation_Stop(this->state.get());

        if( threads_image[idx].joinable() )
            threads_image[idx].join();
        else if( thread_chain.joinable() )
            thread_chain.join();
    }
    else // Start method
    {
        int idx = System_Get_Index(this->state.get());
        selected_method = static_cast<Method>( method_select->selectedIndex() );
        selected_solver = static_cast<Solver>( solver_select->selectedIndex() );

        // Join either the current image thread or the chain thread
        if(selected_method == Method::GNEB)
        {
            if (thread_chain.joinable())
                thread_chain.join();
            this->thread_chain = std::thread(&Simulation_GNEB_Start, this->state.get(), selected_solver, -1, -1, false, -1);
        } else {
            if (threads_image[idx].joinable())
                threads_image[idx].join();

            if (selected_method == Method::LLG )
                this->threads_image[idx] = std::thread(&Simulation_LLG_Start, this->state.get(), selected_solver, -1, -1, false, -1, -1);

            else if (selected_method == Method::MC)
                this->threads_image[idx] = std::thread(&Simulation_MC_Start, this->state.get(), -1, -1, false, -1, -1);

            else if (selected_method == Method::MMF)
                this->threads_image[idx] = std::thread(&Simulation_MMF_Start, this->state.get(), selected_solver, -1, -1, false, -1, -1);
        }
    }
}

void MethodWidget::draw(NVGcontext *ctx)
{
    if( Simulation_Running_On_Image(this->state.get(), idx_img, idx_chain) )
    {
        start_stop_button->setCaption("Stop");
        start_stop_button->setIcon(ENTYPO_ICON_CONTROLLER_PAUS);
    }
    else
    {
        start_stop_button->setCaption("Start");
        start_stop_button->setIcon(ENTYPO_ICON_CONTROLLER_PLAY);
    }
    Window::draw(ctx);
}