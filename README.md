# Distributed System for Presidential Election

### This is a project done during the course [Distributed Systems Programming](https://github.com/howlettjames/Distributed-Systems-Programming "Repository") taken at ESCOM - IPN. The purpose of this project was to learn how to design and implement a Distributed System in C++ language. It was accomplished succesfully having met all the Criteria and thus achieving a 10/10 grade in the subject.

## Task

Design and implement a prototype of a distributed system to tackle down the problem of register, store and show in a web page the statistics of the votes received from a fictional text messaging service (SMS) national server.

## Tech Stack
* C++
* C
* Javascript
* Materialize
* HTML 5
* CSS
* User Datagram Protocol (UDP)

## Criteria

* Taking note of the Request-Reply Protocol described in "Distributed Systems" by George Colouris 5 Edition [Chapter 5](https://www.slideshare.net/lara_ays/chapter-5-slides-15656621) implement a reliable version (against packet loss), Object Oriented approach and adapt it according to the project needs using the User Datagram Protocol (UDP).

* The System must acknowledge each vote received by sending back a message with an unique timestamp including the exact moment in which the vote was stored in the server.

* The System has to check whether a vote was already been done. This must be accomplished by maintaining a TRIE data structure which has a record of all cellphones that had already participated.

* Because it is a distributed system use 3 servers to do all the task. This involves having a balance and a communication mechanism among the servers.

* Last, the system must have a web interface i.e. a web page in which the administrators can check the statistics about all the votes registered in real time. This involves that each of the servers will have a web server embedded, use [Mongoose](https://cesanta.com/docs/overview/intro.html) for this task.
