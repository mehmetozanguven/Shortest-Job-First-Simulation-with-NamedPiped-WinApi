# Shortest-Job-First-Simulation-with-NamedPiped-WinApi
Simulation for multiprocesses using Shortest Job First algorithm on Windows Environment. 
Communication between parent process and child processes are making by named piped.

To run correctly:
1) Open Visual Studio
2) Create New Empty Project named the Parent(File > New > Project)
3) Go to Source Files, then right click to New Item named parent.c,
4) Copy the parent.c from repository to your project
5) In your project, click the Solution "ProjectName", then go to Add, then go to New Project (Solution > Add > New Project)
6) Create New Empty Project named the Child(BE CAREFUL!! The project name is case sensitive, project name as child or other things give error(s))
7) In the Child Project, go to Source Files, then right click to New Item named child.c
8) Copy the child.c from repository to Child project
9) DO NOT FORGET! Right click to Parent project then click "Set as StartUp Project"
10) Build the solution and run.
