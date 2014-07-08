# Depends on some environment variables to build:
# Mac examples:
#
# 488045-mitll:thunderdome go22670$ echo $JAVA_HOME
# /Library/Java/JavaVirtualMachines/jdk1.7.0_25.jdk/Contents/Home
# and to run:
# 488045-mitll:thunderdome go22670$ echo $DYLD_LIBRARY_PATH 
# /Library/Java/JavaVirtualMachines/jdk1.7.0_25.jdk/Contents/Home/jre/lib/server:

./example_client_java lib/CommunityDetectionJar-bin.jar:lib/runtime_2.10.jar:lib/scala-library-2.10.3.jar:out:lib/la4j-0.4.9.jar:lib/commons-lang3-3.3.2.jar CommunityDetectionTest
