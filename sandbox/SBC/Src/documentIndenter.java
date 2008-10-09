import java.io.*;
import java.util.*;


public class documentIndenter{

	String fpath;
	ArrayList rawfile = new ArrayList();

	public static void main(String[]args){

	}

	public documentIndenter(String fpath){
		this.fpath=fpath;
		load();
		save();
	}
	private void load(){
		File file9 = new File(fpath);
		BufferedReader br1, br2,br3, br4;
		try{
			br1 = new BufferedReader(new InputStreamReader(new FileInputStream(file9)));
			br2 = new BufferedReader(new InputStreamReader(new FileInputStream(file9)));
			br3 = new BufferedReader(new InputStreamReader(new FileInputStream(file9)));
			br4 = new BufferedReader(new InputStreamReader(new FileInputStream(file9)));
		}
		catch(FileNotFoundException ex){
			System.exit(0);
			return;
		}

		int i = 0;

		String s = "";
		int ssize = 0;
		String s1 = "";
		String s2 = "";
		String temp;
		try{
			while(br2.readLine() != null){
				rawfile.add(br1.readLine().trim());

			}


		}
		catch(IOException ex){
			return;
		}

	}

	public void save(){
		File file9 = new File(fpath);

		PrintWriter printwriter;
		File file = file9;
		try{
			printwriter = new PrintWriter(new BufferedOutputStream(new FileOutputStream(file)));
		}
		catch(IOException ioexception){
			System.out.println(ioexception);
			System.out.println(file);
			System.out.println(file.canRead());
			System.out.println(file.canWrite());
			return;
		}

		String t1 = "";
		String t2 = "";
		String t3 = "";
		int rsize = rawfile.size();
		int tabs = 0;

		for(int i=0;i<rsize;i++){
			String temp2 = (String)rawfile.get(i);
			String temp = temp2.trim();
			if(!temp.equalsIgnoreCase("end")){
				for(int i2=0;i2 != tabs;i2++){
						printwriter.print("\t");
				}
			}


			if(temp.startsWith("function") || temp.startsWith("if") || temp.startsWith("for") || temp.startsWith("while")){
				if(!temp.startsWith("--"))
					tabs++;

			}
			if(temp.equalsIgnoreCase("end")){
				if(tabs > 0)
					tabs--;

				for(int i2=0;i2 != tabs;i2++){
					printwriter.print("\t");
				}
			}

			printwriter.println(temp);
		}



		printwriter.close();

	}

}