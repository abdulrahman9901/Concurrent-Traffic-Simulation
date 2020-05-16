#include <iostream>
#include <random>
#include "TrafficLight.h"


template <typename T>
T MessageQueue<T>::receive()
{

    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable

    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{


    std::lock_guard<std::mutex> uLock(_mutex);

    _queue.push_back(std::move(msg));
    _cond.notify_one(); // notify client after pushing new TrafficLightPhase into vector
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{

    // initalize variables
    int cycleDuration = 1; // duration of a single simulation cycle in ms
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;

    // init stop watch
    lastUpdate = std::chrono::system_clock::now();
    while (true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= cycleDuration)
        {
            if (_myQueue.receive() == TrafficLightPhase::green)
                return;
        }
    }

}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{

    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{

    // initalize variables
    std::random_device dev;  // https://stackoverflow.com/a/13445752
    std::mt19937 rng(dev()); // Random number generator
    std::uniform_int_distribution<std::mt19937::result_type> distribution(4000, 6000); // dist betweeen 4000ms and 6000ms
    int lastCycleDuration = distribution(rng); // duration of a single simulation cycle in ms
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;

    // init stop watch
    lastUpdate = std::chrono::system_clock::now();
    while (true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= lastCycleDuration)
        {
            // change the state of the traffic light
            _currentPhase = _currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red;

            // Sends message to message queue
            _myQueue.send(std::move(_currentPhase));

            // reset stop watch for next cycle
            lastUpdate = std::chrono::system_clock::now();
            lastCycleDuration = distribution(rng); // Update cycle duration
        }
    }

}
