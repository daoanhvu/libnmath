package test;
import java.io.FileOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;
import java.nio.channels.FileChannel;
import java.io.File;

public class Test {
	static {
		//System.loadLibrary("D:\\Documents\\testapp\\test\\test64.dll");
		System.loadLibrary("test64");
		//System.loadLibrary("test32");
	}
	
	public static void main(String[] args) {
		int len;
		try{
			String ch = "a" + Character.toString((char)0x2201);
			//byte[] buf = ch.getBytes("UTF8");
			//System.out.println("String: " + ch + "; byte[2]: " + buf[2] + "; String len:" + ch.length() + "; length of buff: " + buf.length + " byte(s)");
			len = jniWriteString("D:\\data.dat", ch);
			
			System.out.println("bytes written: " + len);
		}catch(Exception ex){
			ex.printStackTrace();
		}
	}
	
	static native int jniWriteString(String fileName, String data);
}