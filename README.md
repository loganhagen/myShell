<h1>myShell</h1>

<p>BEFORE RUNNING: Please ensure profile file is located at ~/.myShell_profile, or default environment variables will be used.

COMPILATION: Enter "make" to build the myShell executable in the current directory.

NOTES:
        - The shell uses a space (" ") as a delimiter. Not using a space between tokens will produce unpredictable behaviour.
        - Background processes are kept track of in a linked list. I use a linked list API provided by Denis Nikitenko, instructor of CIS-2750 this term. 
        - Unlimited amount of input or output redirection operators is allowed (ex. "ls > out1 > out2 > out3").
        - myPATH, myHISTFILE and myHOME have been implemented.
        - "cd", "history", "export" have been implemented.
        - "export $myPATH" never replaces the value of myPATH, only appends. 
        - "export $myHOME" and "export $myHISTFILE" always replace their respective values.
        - "echo" implemented with limited capability. Can be used to see values of environment variables (ex. "echo $myPATH").</p>