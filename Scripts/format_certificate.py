'''
Python Script to format certificates

'''

import os
import sys

path = os.path.dirname(os.path.realpath(__file__))

#Function that adds a new line character and trailing backslash except on the final line
def add_newline(f):
    
    with open(f, 'r') as fd:
        lines = fd.read().splitlines()
 
    line_num = 0
    for i in lines:
        i = "\""+i+"\\n\""
        line_num = line_num + 1
        if(len(lines) == line_num):
            print(i)
        else:
            print(i+"\\")

#Main function. Execution starts here
if __name__ == '__main__':

    if len(sys.argv) == 1:
        filename = input("Enter Filename (you can also enter the filename on the command line): ")
    else:
        filename = sys.argv[1]

    add_newline(filename)
    print ("")
            
# [] END OF FILE
