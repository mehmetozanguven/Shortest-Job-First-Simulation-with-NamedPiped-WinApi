//Mehmet Ozan Güven - 220201036
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#pragma warning(disable:4996)//avoid error:_CRT_SECURE_NO_WARNINGS

#define NO_OF_PROCESS 5
//define pipes name same as the parent's pipes name.
char *pipesNames[NO_OF_PROCESS] = { "\\\\.\\Pipe\\pipe1", "\\\\.\\Pipe\\pipe2", "\\\\.\\Pipe\\pipe3" , "\\\\.\\Pipe\\pipe4" , "\\\\.\\Pipe\\pipe5" };
//define buffer size to determine size of the message
#define BUFFER_SIZE 1024 
//childID determines which child, random_t is random number that generates by that child
int childID, random_t;
//define pipe handlere to connect with parent via pipe
HANDLE pipeHandler;

void connectEachChildToParent(int childID);
void generateRandomNumber();
int readFromParent();
void writeToParent(char *messageToParent);
void write_t_n_ToParent();
void executionCycle();

/*
	First, check child has 2 argument,
		If not exit 
		If it has, get childID(convert to int using atoi )
			connect that child to parent
			then go to executionCycle
*/
int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("Error in child process...now exiting %d\n", argv[0]);
		system("pause");
		exit(0);
	}
	else
	{
		childID = atoi(argv[1]);
		printf("Child %d space\n\n", childID + 1);
		connectEachChildToParent(childID);
		executionCycle();	
	}
	system("pause");
	CloseHandle(pipeHandler);
	return 1;
}

/*
	5 rounds.
	Each round first read parent message
	If message corrects (readFromParent returns 1),
		then, say to parent that "I'm working on my execution"
		generate  random number
		send to random number to "I'm done on my execution"
		then say to parent that done
	If not,
		then, warn to user.
*/
void executionCycle()
{
	for (int i = 0; i < 5; i++)
	{
		if (readFromParent() == 1)
		{
			writeToParent("I'm working on my execution");
			generateRandomNumber();
			Sleep(random_t);
			write_t_n_ToParent();
			writeToParent("I'm done on my execution");
		}
		else
		{
			printf("Parent have send different message\n");
		}
		
	}
}

/*
	This method does : Connect to the parent via parameter.
	@param childID is determines which child connect with pipe

*/
void connectEachChildToParent(int childID)
{
	pipeHandler = CreateFile(
		pipesNames[childID],   // pipe name 
		GENERIC_READ |  // read and write access 
		GENERIC_WRITE,
		0,              // no sharing 
		NULL,           // default security attributes
		OPEN_EXISTING,  // opens existing pipe 
		0,              // default attributes 
		NULL);          // no template file 

	if (INVALID_HANDLE_VALUE == pipeHandler)
	{
		printf("Error occurred while connecting to the parent.\n");
		CloseHandle(pipeHandler);
		return; 
	}
	
}

/*
	This function does : Generate different random number for each child processes.
	I added some different arbitrary number to the srand for each child
	Because if I would use just srand(time(NULL)), then all child process
	would have same random number. Because, it is the same time for all child processes
*/
void generateRandomNumber()
{
	switch (childID)
	{
	case 0:
		srand(time(NULL) + 20);
		random_t = rand() % 251 + 50;
		break;
	case 1:
		srand(time(NULL) + 13);
		random_t = rand() % 251 + 50;
		break;
	case 2:
		srand(time(NULL) + 23);
		random_t = rand() % 251 + 50;
		break;
	case 3:
		srand(time(NULL) + 28);
		random_t = rand() % 251 + 50;
		break;
	case 4:
		srand(time(NULL) + 37);
		random_t = rand() % 251 + 50;
		break;
	default:
		printf("Invalid command Line\n");
		exit(0);
	}

}

/*
	This method does : Reading message from parent. 
	If message is the correct message(says that start working)
		then returns 1
	If not 
		then returns 0


	If result of ReadFile is false, 
		then I print the warning message and 
		close the handle, returns 0
	If not,
		then message that receives is stored the char messageToReceive
		and returns 1
*/
int readFromParent()
{
	char messageToReceive[BUFFER_SIZE];
	DWORD cbBytes;
	BOOL bResult = ReadFile(
		pipeHandler,
		messageToReceive,
		sizeof(messageToReceive),
		&cbBytes,
		NULL
	);
	if ((!bResult) || (0 == cbBytes))
	{
		printf("Error occurred while reading from parent process.\n");
		CloseHandle(pipeHandler);
		return 0;  
	}
	else if (strncmp(messageToReceive, "Do your execution", 17) == 0)
	{
		return 1;
	}
	else
	{
		return  0;
	}
}

/*
	This method does : Sending a signal to parent to alert when child process starts work and ends work.(via parameter)
	@param messageToParent is a message to the parent that determines child process is working or is ending its works.
	
	I am sending a message to parent using WriteFile()
	If WriteFile() returns False
		then warn to user and close the handle
	If not
		then, that's means message has send successfuly.
*/
void writeToParent(char *messageToParent)
{
	DWORD cbBytes = 0;

	BOOL bResult = WriteFile(
		pipeHandler,
		messageToParent,
		strlen(messageToParent) + 1,
		&cbBytes,
		NULL);

	if ((!bResult) || (strlen(messageToParent) + 1 != cbBytes))
	{
		printf("Error occurred while writing to parent process.\n");
		CloseHandle(pipeHandler);
		return;
	}
}

/*
	This function does : Sending a random tn number to the parent via WriteFile()
	
	First, I convert to int random tn number to char array, then I am sending to the parent
	If WriteFile() returns false,
		then warn to user and close the handle
	If not
		then, that's means message has send successfuly
*/
void write_t_n_ToParent()
{
	char tn_number[100];
	sprintf(tn_number, "%d", random_t);
	DWORD cbBytes = 0;

	BOOL bResult = WriteFile(
		pipeHandler,
		tn_number,
		strlen(tn_number) + 1,
		&cbBytes,
		NULL);

	if ((!bResult) || (strlen(tn_number) + 1 != cbBytes))
	{
		printf("\nError occurred while writing to the parent process.\n");
		CloseHandle(pipeHandler);
		return; 
	}
	
}
