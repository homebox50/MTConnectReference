# MTConnectReference

Some terminology

MTCONNECT is a manufacturing technical standard to retrieve process information from numerically controlled machine tools. Some terms: 

Adapter: An optional software component that connects the Agent to the Device. 

Agent: A process that implements the MTConnect® HTTP protocol, XML generation, and MTConnect protocol. 

Device: A piece of equipment capable of performing an operation. A device may be composed of a set of components that provide data to the application. The device is a separate entity with at least one component or data item providing information about the device.

Application (client): the application is the actual requestor and consumer of MTConnect data. Typical 
functions of the application are to request, store, manipulate and display data. The Application includes 
a function called the Client whichinitiates all requests for MTConnect data. The Client is a software 
function in the application that actually requests data from the Agent and translates that data into the 
format required for the application. 

Relevent links:

https://github.com/mtconnect/adapter

https://github.com/mtconnect/cppagent

https://github.com/mtconnect/py_agent

http://mtcup.org/wiki/Main_Page

https://www.youtube.com/watch?v=3Il-EYu_XxY&t=
https://www.mtconnect.org/

http://mtconnectforum.com/Default.aspx

Those wishing to get a better feel for MTConnect should check out the lab directory; there is a readme there that should walk you through some basics, or watch the youtube link above. Additionally, this may be a good starting point for creating our own MTConnect platforms
