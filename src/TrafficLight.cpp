#include <iostream>
#include <random>
#include <thread>
#include <deque>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> lock(_mtx);
    _cond.wait(lock, [this] { return !_queue.empty(); });
    return std::move(*_queue.begin());
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lockGuard(_mtx);
    _queue.clear();
    _queue.emplace_back(std::move(msg));
    _cond.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true) {
      if (_queue.receive() == TrafficLightPhase::green) return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    this->threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    
    // Required setup for generating a random number between 400 and 6000
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distr(4000, 6000);
    
    // Init variable for start of current cycle
    std::chrono::system_clock::time_point cycleStart = std::chrono::system_clock::now();
  
    // Generate a random time duration between 4 and 6 seconds
    int cycleDuration = distr(gen);
    
    while(true) {
      // Get time point for now
      std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
      
      // Get time since cycle started
      long int diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - cycleStart).count();
      
      if (diff > cycleDuration) {
        // Switch light phase
        _currentPhase = _currentPhase == TrafficLightPhase::red ? TrafficLightPhase::green : TrafficLightPhase::red;
        
        // Generate a new random time duration between 4 and 6 seconds for the next cycle
        cycleDuration = distr(gen);
        
        // Reset the cycle start time (as we're starting a new cycle)
        cycleStart = std::chrono::system_clock::now();
        
        // Send an update method to the message queue using move semantics
        _queue.send(std::move(_currentPhase));
      }
      
      // Sleep for 1ms
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}