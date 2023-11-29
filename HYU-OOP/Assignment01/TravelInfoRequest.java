import java.util.Scanner;
import java.util.StringTokenizer;
import java.util.Calendar;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;



public class TravelInfoRequest {
	public static void main(String[] args)  {
		
		KeyValue[] kvArr = new KeyValue[7];
		
		int i = 0;
		
		
		try {
			Scanner proSC = new Scanner(new FileInputStream("properties.txt"));
			Scanner temSC = new Scanner(new FileInputStream("template_file.txt"));
			FileWriter fw = new FileWriter("output.txt");
			
			
			while(proSC.hasNext()) {
				String curLine = proSC.nextLine();
				kvArr[i++] = new KeyValue(curLine);
			}
			
			Calendar calendar = Calendar.getInstance();
			
			String curTime = Integer.toString(calendar.get(Calendar.YEAR)) + "-" 
							+ Integer.toString(calendar.get(Calendar.MONTH)) + "-" 
							+ Integer.toString(calendar.get(Calendar.DATE));
			

			kvArr[i] = new KeyValue("{date}", curTime);
			
			String curLine = null;
			
			while(temSC.hasNext()){
				
				curLine = temSC.nextLine();
				String toWriteLine = "";
				
				
				if(curLine.equals("<add info>")) {
					
					Countries start= null;
					Countries depart = null;
					
					for(int j = 0; j < 7; j++) {
						if((kvArr[j].getKey()).equals("{startcountry}"))  
							start = new Countries(kvArr[j].getValue());
						
						if((kvArr[j].getKey()).equals("{departcountry}")) 
							depart = new Countries(kvArr[j].getValue());
						
					}
					
				
					Distance startD = new Distance(start.getCountry(), start.getLat(), start.getLng());
					Distance departD = new Distance(depart.getCountry(), depart.getLat(), depart.getLng());
					curLine = "Distance of\n" + Distance.getDistance( startD, departD);
					
					
					fw.write(curLine);
					continue;
				}
				
				
				
				StringTokenizer strTok = new StringTokenizer(curLine);
				String curStr = null;
				
				
				while(strTok.hasMoreTokens()) {
					curStr = strTok.nextToken();
					
					
					if(curStr.indexOf('{') != -1) {
						for(int j = 0; j < 7; j++) {
							if(kvArr[j].getKey().equals(curStr.substring(curStr.indexOf('{'), curStr.indexOf('}')+1))) {
								curStr = curStr.replace(curStr.substring(curStr.indexOf('{'), curStr.indexOf('}')+1), kvArr[j].getValue());
								break;
							}
						}
					}
					
						toWriteLine += curStr;
						
						if(strTok.hasMoreTokens())
							toWriteLine += " ";			
				}
				
				fw.write(toWriteLine);
				
				if(temSC.hasNext())
					fw.write("\n");
			}
			
			proSC.close();
			temSC.close();
			fw.close();
			
			
		} catch (FileNotFoundException e) {
			e.getStackTrace();
		} catch (IOException e2) {
			e2.getStackTrace();
		}
		
	}
}
