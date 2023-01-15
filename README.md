# restaurantSimulation
## Project made for OS class, NKUA, winter semester 2016-2017
Aim is to put in practice the concepts of critical section, race condition and semaphores

The different types of prcesses are:
* restaurant 
* waiter
* doorman
* customer

The restaurant process (main), operates on a piece of shared memory, that is the restaurant space. 
Customers are waiting to be served either in the bar, or outside the restaurant, and they are forming queues to do so. A doorman allows customers to enter the restaurant, either to sit on a table, or wait at the bar. Customers may stay for an arbitrary amount of time.

