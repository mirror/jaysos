#!/bin/sh
javac waba/io/*.java waba/fx/*.java waba/lang/*.java waba/sys/*.java waba/ui/*.java waba/util/*.java -d ../classfiles
mv ../classfiles/java/lang ../classfiles/waba/lang
