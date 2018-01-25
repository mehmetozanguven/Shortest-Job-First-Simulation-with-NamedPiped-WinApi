//Mehmet Ozan Güven - 220201036
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#pragma warning(disable:4996)//avoid error:_CRT_SECURE_NO_WARNINGS
//Define buffer size and child process number
#define BUFFER_SIZE 1024 
#define NO_OF_PROCESS 5

//define a and formula to next burst time
#define a 0.5
#define next_burst_time(tn, Tn) (a*tn + (1 - a)*Tn)

//Define pipes name and child command line(number identifies the childID)
char *pipesNames[NO_OF_PROCESS] = { "\\\\.\\Pipe\\pipe1", "\\\\.\\Pipe\\pipe2", "\\\\.\\Pipe\\pipe3" , "\\\\.\\Pipe\\pipe4" , "\\\\.\\Pipe\\pipe5" };
char *commandLine[NO_OF_PROCESS] = { "Child.exe 0", "Child.exe 1" , "Child.exe 2" ,"Child.exe 3" ,"Child.exe 4" };

/*
	Define pipe and process handlers
	Define necessary information to create processes
*/
HANDLE pipeHandlers[NO_OF_PROCESS];
STARTUPINFO si[NO_OF_PROCESS];
PROCESS_INFORMATION pi[NO_OF_PROCESS];
HANDLE processHandles[NO_OF_PROCESS];
SECURITY_ATTRIBUTES sa[NO_OF_PROCESS];

/*
	Define initial burst time and
	Define 5x4 matrix, for this matrix each row specify child processes information
	and column 0 is childID
		column 1 is burst time
		column 2 is random tn number
		column 3 is next burst time
*/
double processInfo_SchedulerOrder[NO_OF_PROCESS][4];
double inital_burst_time[NO_OF_PROCESS] = { 300, 220, 180, 45, 255 };

void waitForConnectionToPipe(int whichPipeForWhichChild);
void fillInitialValue();
void createProcessAndPipes();
void SJFSchedular();
void printExecutionOrder();
void writeToChildProcess();
void WaitForStartSignal(int workerID);
void WaitForEndSignal(int workerID);
void TakeRandom_t_n_Number(int workerID);
int findIndexOfChildProcess(int workerID);
void calculate_new_burst_time();
void printTable(int tableID);
void updateSchedulerAndBurstTime();
void closeHandles();
void executionCycle();

/*
	First create processes and pipes
	then fill the initial burst time with correspong child processes
	then run the steps
	Wait for all processes work are done
	Finally close handles
*/
int main(int argc, char* argv[])
{
	createProcessAndPipes();
	fillInitialValue();
	executionCycle();
	WaitForMultipleObjects(NO_OF_PROCESS, processHandles, TRUE, INFINITE);
	closeHandles();
	system("pause");
	return 1;
}

/*
	This method does : run each steps
	For each steps:
		1 - Schedule the processes order
		2 - print the ordering
		3 - write to child process - to start child works-
		4 - calculate new burst time for each child processes
		5 - print necessary tables that homework wants
		6 - update scheduler and burst time for new execution
*/
void executionCycle()
{
	for (int i = 0; i < 5; i++)
	{
		printf("\n\n\nEXECUTION %d START\n\n\n", i + 1);
		SJFSchedular();
		printExecutionOrder();
		writeToChildProcess();
		calculate_new_burst_time();
		printTable(0);
		printTable(1);
		printTable(2);
		updateSchedulerAndBurstTime();
		printf("\n\n\nEXECUTION %d STOP\n\n\n", i + 1);
	}
}

/*
	This method does : creating child processes and pipes
	I have created named pipe
	After creating named pipe
	wait for connection to child processes
*/
void createProcessAndPipes()
{
	for (int i = 0; i < NO_OF_PROCESS; i++)
	{//for-1
		SecureZeroMemory(&sa[i], sizeof(SECURITY_ATTRIBUTES));
		sa[i].bInheritHandle = TRUE;
		sa[i].lpSecurityDescriptor = NULL;
		sa[i].nLength = sizeof(SECURITY_ATTRIBUTES);

		SecureZeroMemory(&si[i], sizeof(STARTUPINFO));
		si[i].cb = sizeof(STARTUPINFO);

		SecureZeroMemory(&pi[i], sizeof(PROCESS_INFORMATION));
		
		if (!CreateProcess(
			NULL,
			commandLine[i],
			NULL,
			NULL,
			FALSE,
			CREATE_NEW_CONSOLE,
			NULL,
			NULL,
			&si[i],
			&pi[i]))
		{
			printf("Uable to create process : %d\n", i);
			system("pause");
			ExitProcess(0);
		}
		else
		{
			processHandles[i] = pi[i].hProcess;
			printf("Parent has created process number %d\n", i+1);
		}

		pipeHandlers[i] = CreateNamedPipe(
			pipesNames[i],             // pipe name 
			PIPE_ACCESS_DUPLEX,       // read/write access 
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			BUFFER_SIZE,              // output buffer size 
			BUFFER_SIZE,		// input buffer size 
			NMPWAIT_USE_DEFAULT_WAIT, // client time-out 
			NULL);                    // default security attribute 

		if (INVALID_HANDLE_VALUE == pipeHandlers[i])
		{
			printf("Error occurred while creating the pipe");
			CloseHandle(pipeHandlers[i]);
			return;  
		}
		else
		{
			printf("Named pipe created succefully\n.");
		}
		
		waitForConnectionToPipe(i);

	}//for-1
}

/*
	This method does : connect to child to the pipe
	@param whichPipeForWhichChild determines which child will be connected which pipe.
	For example : for child 0
	I know that for named pipe child 0 is holding by pipeHandlers[0] which includes the pipe name \\\\.\\Pipe\\pipe1
	then, in child.c, when child.exe 0 starts, it connects the same pipe name \\\\.\\Pipe\\pipe1, therefore connection succeed.
	If connection failed
		Then warn to user and close the handle
*/
void waitForConnectionToPipe(int whichPipeForWhichChild)
{
	printf("Named pipe is waiting to connect with child\n.");
	BOOL bClientConnected = ConnectNamedPipe(pipeHandlers[whichPipeForWhichChild], NULL);
	if (FALSE == bClientConnected)
	{
		printf("Error occurred while connecting to the client.\n");
		CloseHandle(pipeHandlers[whichPipeForWhichChild]);
		return;
	}
	else
	{
		printf("Connection is done successfully\n.");
	}
}

/*
	This method does : fill up the processInfo_SchedulerOrder array with initial burst time
	and correspond childID
*/
void fillInitialValue()
{
	for (int i = 0; i < NO_OF_PROCESS; i++)
	{
		processInfo_SchedulerOrder[i][0] = i;
		processInfo_SchedulerOrder[i][1] = inital_burst_time[i];
	}
}

/*
	This method does : Shortest first job algoritm. (Using selection algorithm)
	If next child burst time is less than previous one
		then change rows
	If not,
		then, do nothing
*/
void SJFSchedular()
{
	for (int i = 0; i < NO_OF_PROCESS; i++)
	{//for-1
		for (int j = i + 1; j < NO_OF_PROCESS; j++)
		{//for-2
			if (processInfo_SchedulerOrder[i][1] > processInfo_SchedulerOrder[j][1])
			{
				double temp;
				temp = processInfo_SchedulerOrder[i][0];
				processInfo_SchedulerOrder[i][0] = processInfo_SchedulerOrder[j][0];
				processInfo_SchedulerOrder[j][0] = temp;

				temp = processInfo_SchedulerOrder[i][1];
				processInfo_SchedulerOrder[i][1] = processInfo_SchedulerOrder[j][1];
				processInfo_SchedulerOrder[j][1] = temp;

				temp = processInfo_SchedulerOrder[i][2];
				processInfo_SchedulerOrder[i][2] = processInfo_SchedulerOrder[j][2];
				processInfo_SchedulerOrder[j][2] = temp;

				temp = processInfo_SchedulerOrder[i][3];
				processInfo_SchedulerOrder[i][3] = processInfo_SchedulerOrder[j][3];
				processInfo_SchedulerOrder[j][3] = temp;
			}
		}//for-2
	}//for-1
}

/*
	This method does : printing the execution order
*/
void printExecutionOrder()
{
	printf("Execution Order\n");
	printf("<");
	for (int i = 0; i < NO_OF_PROCESS; i++)
	{
		printf("%.1lf, ", processInfo_SchedulerOrder[i][0] + 1);
	}
	printf(">\n");
}

/*
	This method does : writing child pipe according to scheduler order
	Firstly, determine which child works first(processInfo_SchedulerOrder array has already sorted)
	workerID is saying that which child will be starting its works.(according to childID)
	Then, send a message to child to start its work "Do your execution"
	To write a correct pipe, I have choose corresponding handler from pipeHandlers array
	If result of WriteFile() is false,
		then warn to user and close the handle
	If not,
		then wait for start signal, tn number and end signal from parent

*/
void writeToChildProcess()
{
	for (int i = 0; i < NO_OF_PROCESS; i++)
	{//for-1
		int worker_ID = processInfo_SchedulerOrder[i][0];
		
		char messageToSend[BUFFER_SIZE] = "Do your execution";
		DWORD cbBytes = 0;
		BOOL bResult = WriteFile(
			pipeHandlers[worker_ID],                // handle to pipe 
			messageToSend,             // buffer to write from 
			strlen(messageToSend) + 1,   // number of bytes to write, include the NULL 
			&cbBytes,             // number of bytes written 
			NULL);                // not overlapped I/O 

		if ((!bResult) || (strlen(messageToSend) + 1 != cbBytes))
		{
			printf("Error occurred while writting to the child process.\n");
			CloseHandle(pipeHandlers[i]);
			return;
		}
		else
		{
			WaitForStartSignal(worker_ID);
			TakeRandom_t_n_Number(worker_ID);
			WaitForEndSignal(worker_ID);
		}
		
	}//for-1
}

/*
	This method does : waiting for starting signal from child process
	@param workerID  specifies which child process is waiting.
	I have stored the message to messageToReceived array
	If message is "I'm working on my execution",
		then it means that correspond child is working and parent prints "child started".
	If not,
		then, if result of ReadFile() is false,
				then, warn to user and close the handle
			  if not
				then, message that comes from child doesn't mean to start . 
*/
void WaitForStartSignal(int workerID)
{
	char messageToReceive[BUFFER_SIZE];
	DWORD cbBytes = 0;
	BOOL bResult = ReadFile(
		pipeHandlers[workerID],
		messageToReceive,
		sizeof(messageToReceive),
		&cbBytes,
		NULL);

	if ((!bResult) || (0 == cbBytes))
	{
		printf("\nError occurred while reading from the child process.\n");
		CloseHandle(pipeHandlers[workerID]);
		return;
	}
	else if (strncmp(messageToReceive, "I'm working on my execution", 27) == 0)
	{
		printf("P%d started\n", workerID + 1);
	}
	else
	{
		printf("Child have send a message that does not mean to start its execution. Therefore program is closing\n");
		exit(0);
	}
}

/*
	This method does : waiting for ending signal from child process
	@param workerID specifes which child process is waiting.

*/
void WaitForEndSignal(int workerID)
{
	char messageToReceive[BUFFER_SIZE];
	DWORD cbBytes = 0;
	BOOL bResult = ReadFile(
		pipeHandlers[workerID],
		messageToReceive,
		sizeof(messageToReceive),
		&cbBytes,
		NULL);
	if ((!bResult) || (0 == cbBytes))
	{
		printf("\nError occurred while reading from the child process\n");
		CloseHandle(pipeHandlers[workerID]);
		return;
	}
	else if (strncmp(messageToReceive, "I'm done on my execution", 24) == 0)
	{
		printf("P%d stopped\n", workerID + 1);
	}
	else
	{
		printf("Child have send a message that does not mean to end its execution. Therefore program is closing\n");
		exit(0);
	}
}

/*
	This method does : takes the random tn number that comes from child process
	@param workerID specifes which child process is waiting.
	After taking tn number, to put correct index in the array, I find the
	childID index (using findIndexOfChildProcess( ) ) according to workerID
*/
void TakeRandom_t_n_Number(int workerID)
{
	char messageToReceive[BUFFER_SIZE];
	DWORD cbBytes = 0;
	BOOL bResult = ReadFile(
		pipeHandlers[workerID],
		messageToReceive,
		sizeof(messageToReceive),
		&cbBytes,
		NULL);

	if ((!bResult) || (0 == cbBytes))
	{
		printf("\nError occurred while reading from the child process.\n");
		CloseHandle(pipeHandlers[workerID]);
		return;
	}
	else
	{
		int indexOfChild = findIndexOfChildProcess(workerID);
		processInfo_SchedulerOrder[indexOfChild][2] = atoi(messageToReceive);
	}	
}

/*
	This method does : find the correct childID index in the array
	@param workerID specifies which childID index I am looking for.
*/
int findIndexOfChildProcess(int workerID)
{
	int index = 0;
	for (int i = 0; i < NO_OF_PROCESS; i++)
	{
		if (processInfo_SchedulerOrder[i][0] == workerID)
		{
			index = i;
		}

	}
	return index;
}

/*
	This method does : calculate the next burst time for each child processes
*/
void calculate_new_burst_time()
{
	for (int i = 0; i < NO_OF_PROCESS; i++)
	{
		double newBurstTime = next_burst_time(processInfo_SchedulerOrder[i][2], processInfo_SchedulerOrder[i][1]);
		processInfo_SchedulerOrder[i][3] = newBurstTime;
	}
}

/*
	This method does : print each table that homework wants
	@param tableID determines which table prints

*/
void printTable(int tableID)
{
	if (tableID == 0)
	{
		printf("\nProcess		Tn\n");
		for (int i = 0; i < NO_OF_PROCESS; i++)
		{
			printf("%.1lf		%.1lf\n", processInfo_SchedulerOrder[i][0] + 1, processInfo_SchedulerOrder[i][1]);
		}
		printf("\n\n");
	}

	else if (tableID == 1)
	{
		printf("\nProcess		Tn	tn(actual length)\n");
		for (int i = 0; i < NO_OF_PROCESS; i++)
		{
			printf("%.1lf		%.1lf	%.1lf\n", processInfo_SchedulerOrder[i][0] + 1, processInfo_SchedulerOrder[i][1], processInfo_SchedulerOrder[i][2]);
		}
		printf("\n\n");
	}

	else
	{
		printf("\nProcess		Tn	tn(actual length)	T(n+1)\n");
		for (int i = 0; i < NO_OF_PROCESS; i++)
		{
			printf("%.1lf		%.1lf	%.1lf              	%.1lf\n", processInfo_SchedulerOrder[i][0] + 1, processInfo_SchedulerOrder[i][1], processInfo_SchedulerOrder[i][2], processInfo_SchedulerOrder[i][3]);
		}
		printf("\n\n");
	}
}

/*
	Update after getting next burst time 
	change initial burst time with next burst time for all child processes
*/
void updateSchedulerAndBurstTime()
{
	for (int i = 0; i < NO_OF_PROCESS; i++)
	{
		processInfo_SchedulerOrder[i][1] = processInfo_SchedulerOrder[i][3];
		processInfo_SchedulerOrder[i][3] = 0;
	}
}

/*
	This method does : closing the handles
*/
void closeHandles()
{
	for (int i = 0; i < NO_OF_PROCESS; i++)
	{
		CloseHandle(pi[i].hThread);
		CloseHandle(pi[i].hProcess);
		CloseHandle(pipeHandlers[i]);
	}
}