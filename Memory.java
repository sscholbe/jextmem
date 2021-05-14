package extmem;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class Memory implements AutoCloseable {
	static {
		System.loadLibrary("extmem");
	}
	
	/*
	 * Native functions (which are implemented in the library)
	 */
	
	private static native long openProcess(String windowTitle) throws Win32Exception;
	
	private static native void closeProcess(long handle) throws Win32Exception;
	
	private static native byte[] readBytes(long handle, long address, int count) throws Win32Exception;

	private long handle;
	
	public Memory(String windowTitle) throws Win32Exception {
		if(windowTitle == null) {
			throw new IllegalArgumentException("Window title is null");
		}
		handle = openProcess(windowTitle);
		if(handle == -1) {
			throw new Win32Exception("Failed to open process");
		}
	}
	
	@Override
	public void close() throws Win32Exception {
		closeProcess(handle);
	}
	
	public ByteBuffer read(long address, int count) throws Win32Exception {
		if(address < 0 || count < 0) {
			throw new IllegalArgumentException("Negative inputs were provided");
		}
		byte[] data = readBytes(handle, address, count);
		if(data == null) {
			throw new Win32Exception("Failed to read memory");
		}
		// Java uses Little Endian encoding, so we must make sure that we use the correct byte order
		return ByteBuffer.wrap(data).order(ByteOrder.LITTLE_ENDIAN);
	}
	
	public int readInt(long address) throws Win32Exception {
		return read(address, 4).getInt();
	}
	
	public float readFloat(long address) throws Win32Exception {
		return read(address, 4).getFloat();
	}
	
	public long readLong(long address) throws Win32Exception {
		return read(address, 8).getLong();
	}	
}
