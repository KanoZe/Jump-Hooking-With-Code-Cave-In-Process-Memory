This code does jump call hooking and writes a code cave within the game proces's memory at empty intruction addresses for the game "The Battle of wesnoth".
Code uses relative addresses to main game exe module to bypass ASLR.  
Code uses pointer offsets to get to the gold address of the game. 
Compile the cpp to exe and run it, MVSC runs with no errors for sure.
