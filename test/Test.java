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
		//System.loadLibrary("test32");
		System.loadLibrary("test64");
	}
	
	public static void main(String[] args) {
		int len;
		try{
			String ch = "1 + " + Character.toString((char)0x03C0);

			len = jniWriteString("D:\\data.dat", ch);
			//len = writeString("D:\\data.dat", ch);
			
			System.out.println("bytes written: " + len);
		}catch(Exception ex){
			ex.printStackTrace();
		}
	}
	
	static native int jniWriteString(String fileName, String data);

	private static int writeString(String fileName, String data) throws IOException {
		FileOutputStream fos = null;
		int result = 0;
		try{
			fos = new FileOutputStream(new File(fileName));
			byte[] buffer = data.getBytes("utf-8");
			result = buffer.length;
			fos.write(buffer, 0, buffer.length);
			fos.flush();
		}catch(IOException ex) {
			ex.printStackTrace();
		}finally{
			if(fos != null) fos.close();
		}

		return result;
	}
}