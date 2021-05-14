# jextmem
This is an incomplete part of an old project and can not be compiled as such. It allows to read memory of an external process from Java using the Java Native Interface (JNI). For a quite a long time, Anti-Cheats in video games struggled with detecting Java-based cheats. This code demonstrates how such a cheat could access memory outside of the VM using the JNI. Using the `Memory` this becomes as easy as

```java
try (Memory mem = new Memory("Battlefield 4")) {
  // ...
  long clientContext = mem.readLong(0x142670D80L);
  // ...
}
```

Behind the scenes, the library part (.h, .cpp) is responsible for calling the appropriate Win32 functions such as `OpenProcess` and `ReadProcessMemory`. The connection to the Java  code is via the `native` keyword which lets us call these library functions from the VM.
