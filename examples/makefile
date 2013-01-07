CPPFLAGS = -I .. -Wall -g
QT = C:\dev\qt\4.8.4

all: simple simple++ sentinel dict

qt : qsimple qdict
	
q% : q%.cpp
	cl /nologo /MT /Fe$@ $< /W3 /I.. /I$(QT)\include /I$(QT)\include\QtCore /link $(QT)\lib\QtCore4.lib
