Get into project2 directory 
	cd project2

----------------------------------------------------------------

To compile server.c
	gcc -o server server.c
To compile agent.c
	gcc -o agent agent.c

--------------------------------------------------------------
Port       = whatever port you choose.
serverName = Either the IP of the server or its name.
	     I chose to run my server on Zeus so my server 	     	     name was zeus.cs.txstate.edu
action     = "#JOIN" "#LEAVE" "#LIST" "#LOG"
             They must have "" around them and a # in front.

To run server.c
	./server Port
Example ./server 30000

To run agent.c
	./agent serverName Port action
Example ./agent zezeus.cs.txstate.edu 30000 "#JOIN"

--------------------------------------------------------------

Note** port does not have to be 30000
Note*** In order to delete the log.txt file after you turn off 	the server you must go in and delete everyting 		yourself and then save it. It will keep appending to 		the end.