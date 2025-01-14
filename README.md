Language Design
1. Case Sensitivity 
Director really cares about all the details. In Director the words Scene, SCENE, and scene are considered 3 different identities and are to be used accordingly.

3. End of line
Director’s statements end with a semicolon (;).  
Omitting the semicolon will result in a syntax error.
Example:
scene x assign 5;  ## Correct
scene y assign 6   ## Error: Missing semicolon

4. Whitespaces and Indentation
In Director, whitespace characters like spaces, tabs, and newlines are mostly ignored by the compiler, but they serve to separate tokens such as keywords, variables, and operators. While proper indentation enhances the readability of your code, it is not a syntactic necessity. 

5. Comments
Comments are used to explain the code. Director supports both single-line and multi-line comments:  
•	Single-line comment: Begins with ## 
•	Multi-line comment: Enclosed between #* and *# 
Example:
##This is a single-line comment

#*
  This is a multi-line comment.
  It spans multiple lines.
*#

5. Data Type
Variables need to be declared with a type (e.g., scene, frame, script) before they can be used. 

Type	Description	Example
scene	Integer data type	scene age assign 21;
frame	Floating-point number	frame rating assign 4.5;
script	String of text	script dialogue assign "Hello";

6. Control Structure
•	Conditional Statements
cut and cutif
•	cut: Executes a block if the condition is true.
•	cutif: Executes when the cut condition is false, but its condition is true.
Syntax:
cut (condition) {
    ##Code for true condition
}  
cutif (other_condition) {
    ##Code for alternate condition
}
fade {
    ##For prompt and show statements only
}

Example:
scene x assign 10;

cut (x is less 15) {
    show "X is less than 15";
}
cutif (x is greater 20) {
    show "X is greater than 20";
}
fade {
    prompt "End of Execution";
}

7. Reels
•	Loops
roll 
Executes a block for a specified number of iterations.
Syntax:
roll (scene i assign 0; i is less 10; i assign i + 1) {
    ##Code to execute in each iteration
}
##Check added that in roll loop only one variable should be used

repeat
Continues executing as long as the condition is true.
Syntax:
repeat (condition) {
    ##Code to execute while the condition is true
}

8. Operators
Keyword	Function
trim	Subtraction
join	Addition
amplify	Multiplication
split	Division
splice	Modulus

9. Input/Output
Command	Description	Syntax	Example
show	Displays output on the screen.	<show> <LITERAL>;	show "Hello World!";
		<show> <IDENTIFIER>;	show variableName;
prompt	Takes input from the user and assigns it.	<prompt> <LITERAL>;	prompt "Enter your name:";
		<prompt> <IDENTIFIER>;	prompt userInput;

11. Cut Scenes(comparison) 
Comparison operators are used to compare two values or expressions and determine their relationship. They play a crucial role in controlling the logical flow of a program by enabling decision-making based on conditions. You must have either a variable or value at both left and right side of these operators. Here are the common comparison operators and their roles: 
 
Operator 	Function 
isnt 	Returns true if left is not equal to riight 
Is less 	Returns true if the left value is less than the right value. 
Is greater 	Returns true if the left value is greater than the right value. 
Is less or greater 	Returns true if the left value is less than or greater than the right value. 
Is greater or equal 	Returns true if the left value is greater than or equal to the right value. 
Is equal 	Returns true if both values are equal 
 
11. Logical Operators
Logical operators in this language are used to combine conditions for decision-making: 

 	Function 
and 	Ensures both conditions are true 
or 	Ensures at least one condition is true 
not 	Ensures the condition is not true 
 
12. Assigning Task
Assignment 
The assign operator is used to assign a value to a variable. It functions similarly to the = operator in other programming languages.  
