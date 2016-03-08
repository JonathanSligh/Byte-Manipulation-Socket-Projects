import java.io.*;
import java.net.*;

class UDPServer {
static short checkSum(byte[] msg, int length){
	short sum = 0;
	for (int i = 0; i < length; i++)
	{
		short j = (short) (msg[i] & 0x00FF);
		sum = (short)(sum + j);
		sum = (short)((sum & 0x00FF) + (sum >> 8));
	}
	return sum;
}
	public static void main(String args[]) throws Exception       
	{          
		DatagramSocket serverSocket = new DatagramSocket(Integer.parseInt(args[0]));             
		byte[] receiveData = new byte[1024];             
		byte[] sendData = new byte[1024];             
		while(true)                
		{                   
			boolean invalid = false;
			DatagramPacket receivePacket = new DatagramPacket(receiveData, receiveData.length);                   
			serverSocket.receive(receivePacket);                  
			String sentence = new String(receivePacket.getData(), receivePacket.getOffset(), receivePacket.getLength());
			int dataLength = sentence.length(); 
			byte[] initData = receivePacket.getData();
			
			//0x1234
			int i1 = initData[0];
			int i2 = initData[1];
			//total message length
			int tml = 0;
			tml |= initData[2] & 0xFF;
			tml <<= 8;
			tml |= initData[3] & 0xFF;
			byte[] newMessage = new byte[tml];
			InetAddress IPAddress = receivePacket.getAddress();                   
			int port = receivePacket.getPort();                   
			if (!(i1 == 0x12 && i2 == 0x34 && dataLength >= 9 && checkSum(initData,tml) == 255))
			{
				System.out.println("ERROR\nmagics: " + i1 + i2);
				System.out.println("dataLength: " + dataLength);
				System.out.println("checkSum: " + checkSum(initData,tml));
				byte[] errorMessage = new byte[7];
				errorMessage[6] = 0;
				if(i1 != 0x12 || i2 != 0x34)
				{
					errorMessage[6] += (byte) 4;
				}
				if (dataLength < 9)
				{
					errorMessage[6] += (byte) 1;
				}
				if (checkSum(initData, tml) != 255)
				{
					errorMessage[6] += (byte) 2;
				}
				errorMessage[0] = 0x12;
				errorMessage[1] = 0x34;
				errorMessage[2] = 0;
				errorMessage[3] = 7;
				errorMessage[4] = 0;
				errorMessage[5] = 7;
				errorMessage[4] = (byte)checkSum(errorMessage, 7);
				DatagramPacket sendPacket = new DatagramPacket(errorMessage, 7, IPAddress, port);                   
				serverSocket.send(sendPacket);
			}
			else
			{	
				String hostname = "";
				String hostnameString = "";
				int hostnameCounter = 0;
				for (int i = 7; i < tml; i++)
				{
					int length = initData[i];
					System.out.println("Hostname " + " with " + length + " characters: ");
					hostname = "";
					hostnameCounter++;
					for(int j = 0; j < length; j++)
					{
						i++;	
						System.out.print((char)initData[i]);
						hostname += (char) initData[i];
					}
					InetAddress address = InetAddress.getByName(hostname);
					hostnameString +=address.getHostAddress() + ".";
					System.out.println("");
				}
				byte[] sendArray = new byte[7 + (4*hostnameCounter)];
				sendArray[0] = 0x12;
				sendArray[1] = 0x34;
				sendArray[2] = (byte)((short)(7 + 4*hostnameCounter)>>8 & 0xff);
				sendArray[3] = (byte)((short)(7 + 4*hostnameCounter) & 0xff);
				sendArray[4] = 0;
				sendArray[5] = 7;
				sendArray[6] = initData[6];
				int count = 7;
				String num = "";
				while(hostnameString.length() != 0)
				{
					num = hostnameString.substring(0, hostnameString.indexOf("."));
					sendArray[count] = (byte)(Short.parseShort(num) & 0xff);
					hostnameString = hostnameString.substring(hostnameString.indexOf(".") + 1); 
					count++;
				}
				sendArray[4] = (byte)~checkSum(sendArray, 7 + 4*hostnameCounter);                   
				DatagramPacket sendPacket = new DatagramPacket(sendArray, sendArray.length, IPAddress, port);                   
				serverSocket.send(sendPacket);
			}                
		}       
	}
}
