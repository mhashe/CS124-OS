/* Executes the command after forking and does the io redirection to stdout, 
stdin, or stderr if needed. The io redirection is done after forking, so the 
redirection is unique to this process/command. */
void execute_command(struct command *cmd);

// void test_piping();

