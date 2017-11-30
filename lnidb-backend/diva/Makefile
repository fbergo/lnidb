
all: diva.jar

diva.jar: *.java
	javac *.java
	jar cfm diva.jar manifest.txt *.class diva.png panctl.png check.png slicectl.png tool{1,2}.png view{1,2,3,4}.png colctl.png updown.png detail.png

clean:
	rm -f *.class *.jar *~

