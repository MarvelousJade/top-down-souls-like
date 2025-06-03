#include "Timer.h"
#include <SDL2/SDL_timer.h>

Timer::Timer() {
    m_StartTicks =0;
    m_PausedTicks = 0;
    m_Paused = false;
    m_Started = false;
}

void Timer::start() {
    m_Started = true;
    m_Paused = false;
    m_StartTicks = SDL_GetTicks();
    m_PausedTicks = 0;
}

void Timer::stop() {
    m_Started = false;
    m_Paused = false;
    m_StartTicks = 0;
    m_PausedTicks = 0;
}

void Timer::pause() {
    if( m_Started && !m_Paused ) {
        m_Paused = true;

        //Calculate the paused ticks
        m_PausedTicks = SDL_GetTicks() - m_StartTicks;
        m_StartTicks = 0;
    }
}

void Timer::unpause() {
    //If the timer is running and paused
    if( m_Started && m_Paused ) {
        m_Paused = false;

        //Reset the starting ticks
        m_StartTicks = SDL_GetTicks() - m_PausedTicks;

        m_PausedTicks = 0;
    }
}

Uint32 Timer::getTicks() {
    //The actual timer time
    Uint32 time = 0;

    if( m_Started ) {
        if( m_Paused ) {
            //Return the number of ticks when the timer was paused
            time = m_PausedTicks;
        }
        else {
            //Return the current time minus the start time
            time = SDL_GetTicks() - m_StartTicks;
        }
    }

    return time;
}

bool Timer::isStarted() {
    //Timer is running and paused or unpaused
    return m_Started;
}

bool Timer::isPaused() {
    //Timer is running and paused
    return m_Paused && m_Started;
}
