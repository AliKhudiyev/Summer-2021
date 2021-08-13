echo $1 $2
nasm -fmacho64 $1 && ld $2 -L/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib -lSystem
