

all: lniviewer.jar

JAVAC = javac
JFLAGS = -classpath httpclient-4.4.jar:httpcore-4.4.jar:commons-logging-1.2.jar:commons-codec-1.9.jar:fluent-hc-4.4.jar:commons-lang3-3.3.2.jar
JAR = jar

lniviewer.jar: *.java manifest.txt
	$(JAVAC) $(JFLAGS) *.java
	$(JAR) cvfm lniviewer.jar manifest.txt *.class httpclient-4.4.jar httpcore-4.4.jar commons-logging-1.2.jar commons-codec-1.9.jar fluent-hc-4.4.jar commons-lang3-3.3.2.jar gear.png viewer_icon.png

run:
	java -jar lniviewer.jar

clean:
	/bin/rm -f *.class lniviewer.jar



