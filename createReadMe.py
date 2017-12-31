#!/usr/bin/python
#this is just a quick script to generate the todo section of the README
#may publish this as a separate github project and/or as part of some future corpus of utilities

import re
import glob

readmeName = "README.md"
template = """\
# (WIP) Variable Topology General Use Multithreaded Neural Network

Much of this project is still under heavy development. Currently not even at a useable state.
The overall plan is to have a general use Neural Net framework which will only require the creation of input and output neuron classes in order to be used.

# TODO
{}
"""

todos = {}

for i in glob.glob("*"):
    if re.compile("^[mA-Z].*[ph]$").match(i) != None:
        todos[i] = [];
        with open(i,"r") as f:
            for j in f:
                if "//todo" in j:
                    todos[i].append(j[7:])

todoString = ""
for i in todos.keys():
    todoString += """
{0}
{1}
""".format(i,"\n".join(["\t- {}".format(k) for k in todos[i]]))

print(template.format(todoString))
with open(readmeName,"w") as f:
    f.write(template.format(todoString))

    
