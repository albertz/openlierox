//This is the main class

//The app will retrun 0 if it compiled correctly, -1 if there are warnings but no errors and -2 if there were errors


import java.io.*;
import java.util.*;


public class main{
	String vers = "0.5B";

	String[] args;
	String path;
	boolean saveresult = false;
	boolean gotpath = false;
	ArrayList argsl = new ArrayList();
	String path2 = System.getProperty("user.dir");
	Encrypter encr;


	/*Yes there is an easter egg!
	However I have encrypted all of it with a secret key which i might as well tell you is "hello sailor" without the quotes (Yea i know it's stupid but who'd guess it?).
	To activate eggs run the program with the parameter -iamanegghead and then when it loads type hello sailor. All eggs are quotes form season 1 of Red Vs Blue (http://redvsblue.com).
	You can decrypt all possible statements that you can type using the encrypter class.
	It is a simple class that allows you to encrypt/decrypt a string. Constructer works as follows: Encrypter encr = new Encrypter(encryptionkey); - Encryption key is just a string which in this case is hello sailor. You can then use the methods encr.decrypt(encryptedstring); encr.encrypt(string); to decrypt and encrypt strings.
	So if you want the key's someone who is fluent in java will have to write an app to decrypt strings.
	The encrypted strings are in the method: eggit()
	Hope you have fun!*/

	boolean egg = false;


	boolean go = true;
	int exitcode = 0;

	//This class is used to load and execute main.txt
	mainReader maincfg;

	//This is used to store any errors
	ArrayList errors = new ArrayList();

	//This is used to store any warnings
	ArrayList warnings = new ArrayList();



	public static void main(String[]args){
		main main = new main(args);
	}

	public main(String[]args){
		this.args=args;
		System.out.println("LXE SCRIPT BASIC COMPILER V"+vers+"\n(C) 2007 Nspire Software\\Rye Productions");
		System.out.println();
		if(java.lang.reflect.Array.getLength(args) > 0){
			for(int i=0;i<java.lang.reflect.Array.getLength(args);i++){
				argsl.add(args[i]);
			}
		}

		sortargs();

		if(!gotpath){
			System.out.print("Enter location where mod script files are stored: ");
			String temp = readInput();
			if(egg){
				encr = new Encrypter(temp);
				try{
					if(encr.decrypt("HcjFIcwPXbfD3jVO/hQyPQ==") != null){
						System.out.println(encr.decrypt("RuO+4+0nIpnWzqDc6FUwcQ=="));
						go = false;
						eggit();
					}
				}catch(NullPointerException e){}

			}
			if(go)
				path = temp;
		}/*else{
			for(int i=0;i<java.lang.reflect.Array.getLength(args);i++){
				argsl.add(args[i]);
			}
		}*/
		System.out.println("Reading files from: "+path);
		//Initialising the class used to load and execute main.cfg
		maincfg = new mainReader(path, this);

		exit();
	}
	private void sortargs(){
		int length = argsl.size();
		for(int i=0;i<length;i++){
			String temp = (String)argsl.get(i);
			if(stringStartsWith(temp.trim(), "-")){
				processArg(temp.trim());
			}else{
				path = temp.trim();
				gotpath = true;
			}
		}
	}

	private void eggit(){
		String temp = "";
		boolean flag = true;
		do{
			temp = readInput().toLowerCase();
			if(encr.encrypt(temp).equalsIgnoreCase("Q1lIegcUsBaAAIiDmLiCErTrTr5hs5mfnMmOsLlPWK8=")){
				System.out.println(encr.decrypt("CDJSeLyF9bwvTpgSrKCkbGHg1HcTXWx3"));
			}else if(encr.encrypt(temp).equalsIgnoreCase("qhnOmZbZT5cLwYVdcl3WXqRkBfJvJmqn5IV2WfKbGrZ3RxW516TT9w==")){
				System.out.println(encr.decrypt("ZXYbb2gu4tgwqJQL+a6yGCFvBF5BIDq7Cj1FI37Ja4xZkFyHnkLV6Gb3C/yCj8HivvogR/rcBxo2AVlsUQXb/w=="));
			}else if(encr.encrypt(temp).equalsIgnoreCase("U2uhgNCW0Nk=")){
				System.out.println(encr.decrypt("qfkXoksmZxNgz88VS7SoJ8mHnWEaw/omzyFgbBFgUy0RRPu/9F3DLhSXN3F/t1961WQrAQ7DjLA="));
			}else if(encr.encrypt(temp).equalsIgnoreCase("KEsIbPfqo+c=")){
				System.out.println(encr.decrypt("ytkNkJbwFSgbjFsCRQ0irN1isafEDuUutpHkrOmgB0zsBvDfhsnIKA=="));
			}else if(encr.encrypt(temp).equalsIgnoreCase("c0fvBwvlUf8=")){
				System.out.println(encr.decrypt("k3fNgY/49+ftCKGmAB54ZuEvylhJf5w4zHrxoTs1noqi42hVFhJiMA=="));
			}else if(encr.encrypt(temp).equalsIgnoreCase("E8+PhJhdrg4=")){
				System.out.println(encr.decrypt("no1VVGuf9L7SAS3x4FLKZw=="));
			}else if(encr.encrypt(temp).equalsIgnoreCase("NGE5wCsRJsg=")){
				System.out.println(encr.decrypt("GuV191F2Y4Tt1yJtJOcZD/EtqHWqEA8RKO31GLuUTAPFiIyydybwE8LIZeWgFXQnetq2YRg/WNKPXfD/VNjpgVqvnG6w3PL5E7uiZvNcFcxMbqLUeNAhJJFnL7FZ/EGwKU+mctZhtb2oHpH3oifJB25VopnTlrWrgLPQ5Oh6Udl8kS8x8BSot/Gnl9NWjAO3k6js1AlBrkNdGlKYVy8niAIVsz+LdHyl2OAyT84vBavolP4X234vAjg9LCTqnN18lv5hYTVy5qnJYqgUD7oEdcSVNkttrF3BOPRFqfljY64Hu+YJnBrscKkSDM7d71sXDvj++xHsmm7Xthtl1xVwLWY8k7qcJr+pZerCObSLvWM="));
			}else if(encr.encrypt(temp).equalsIgnoreCase("r7PvhmVrLiGIJPacfC83zg==") || encr.encrypt(temp).equalsIgnoreCase("y5RfnWWeJnU=")){
				System.out.println(encr.decrypt("ogW3aORZLbMZtMr+3l9M/obDGfTuRO3YaSSUM+XvGp8MZebEVNy+0ybgPiJ5glMo"));
			}else if(temp.equalsIgnoreCase("exit")){
				break;
			}
		}while(flag);
		System.exit(0);
	}

	private void processArg(String flag){
		if(flag.equalsIgnoreCase("-savelog")){
			saveresult = true;
		}if(flag.equalsIgnoreCase("-iamanegghead")){
			egg = true;
		}
	}


	public boolean stringStartsWith(String string, String text){
		int tsize = text.length();
		String string2 = string.substring(0,tsize);
		if(text.equalsIgnoreCase(string2)){
			return true;
		}
		return false;


	}

	public void exit(){
		System.out.println("\n\n");
		printBuildInformation();
		System.out.println();
		if(saveresult)
			saveLog();
		System.out.println("Press enter to exit");
		String f = readInput();
		System.exit(exitcode);
	}

	private void printBuildInformation(){
		int errornum = 0;
		int warningnum = 0;
		errornum = errors.size();
		warningnum = warnings.size();
		String tempwar = "Warnings";
		String temperr = "Errors";
		if(warningnum == 1)
			tempwar = "Warning";
		if(errornum == 1)
			temperr = "Error";

		if(errornum > 0){
			for(int i=0;i<errornum;i++){
				String temp = (String)errors.get(i);
				System.out.println("Error: "+temp);
			}

		}
		if(warningnum > 0){
			for(int i=0;i<warningnum;i++){
				String temp1 = (String)warnings.get(i);
				System.out.println("Warning: "+temp1);
			}

		}
		System.out.println();


		if(errornum > 0 || warningnum > 0)
			System.out.println("Tool Completed with "+warningnum+" "+tempwar+" and "+errornum+" "+temperr);

		if(errornum > 0)
			exitcode = -2;

		if(warningnum > 0){
			exitcode = -1;
		}






		if(errornum == 0 && warningnum == 0){
			System.out.println("Tool completed successfully with 0 Warnings and 0 Errors");
			exitcode = 0;
		}


	}

	public void addError(String error){
		errors.add(error);
	}

	public void addWarning(String warning){
		warnings.add(warning);
	}

	private void saveLog(){
		File file9;
		File file1;
		file9 = new File(path2+"\\results.txt");
		file1 = new File(path2+"\\");
		file1.mkdirs();

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

		int errornum = 0;
		int warningnum = 0;
		errornum = errors.size();
		warningnum = warnings.size();
		String tempwar = "Warnings";
		String temperr = "Errors";
		if(warningnum == 1)
			tempwar = "Warning";
		if(errornum == 1)
			temperr = "Error";

		if(errornum > 0){
			for(int i=0;i<errornum;i++){
				String temp = (String)errors.get(i);
				printwriter.println("Error: "+temp);
			}

		}
		if(warningnum > 0){
			for(int i=0;i<warningnum;i++){
				String temp1 = (String)warnings.get(i);
				printwriter.println("Warning: "+temp1);
			}

		}
		printwriter.println();


		if(errornum > 0 || warningnum > 0)
			printwriter.println("Tool Completed with "+warningnum+" "+tempwar+" and "+errornum+" "+temperr);






		if(errornum == 0 && warningnum == 0)
			printwriter.println("Tool completed successfully with 0 Warnings and 0 Errors");

		String t1 = "";
		String t2 = "";
		String t3 = "";










		printwriter.close();
	}

	private static String readInput() {
		try {
			BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
			return in.readLine();
		}
		catch(IOException e){};
		return "";
	}
}

