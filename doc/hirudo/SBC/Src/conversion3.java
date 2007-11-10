import java.io.*;
 /**
 * Number converter V3.1 ©NickSoft 2004
 * This software is provided "AS IS," without a warranty of any kind. ALL
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING
 * ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED. NickSoft AND ITS LICENSORS SHALL NOT
 * BE LIABLE FOR ANY DAMAGES OR LIABILITIES SUFFERED BY LICENSEE AS A RESULT
 * OF OR RELATING TO USE, MODIFICATION OR DISTRIBUTION OF THE SOFTWARE OR ITS
 * DERIVATIVES. IN NO EVENT WILL NickSoft OR ITS LICENSORS BE LIABLE FOR ANY LOST
 * REVENUE, PROFIT OR DATA, OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL,
 * INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY
 * OF LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE SOFTWARE, EVEN
 * IF NickSoft HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * if you have any questions email them to me at nicky_sh@ihug.co.nz

 * this program was made entierley on Text Pad
 */
public class conversion3 {
	static int hh = 1;
	public static void main(String[]args) {
	System.out.println("---------------------");
	System.out.println("---------------------");
	System.out.println("Number converter V3.1");
	System.out.println("---------------------");
	System.out.println("---------------------");
	System.out.println();
	System.out.println();
	do{
	System.out.println("What do you want to convert? (mm cm m km) ");
	String wdyw = readInput();
	if(wdyw.equalsIgnoreCase("exit")) {
		break;
	}
	if(wdyw.equalsIgnoreCase("mm")) {
			  System.out.println("what do you want to convert it to? (cm m km)");
	          String mmwdyw = readInput();
	          if(mmwdyw.equalsIgnoreCase("cm")) {
	          	System.out.print("write ne number of millimetres you want to convert to centermetres:  ");
				double mmtcm = Integer.parseInt(readInput());
				double mmtcmans = (mmtcm / 10);
				System.out.print("this is the answer:  ");
				System.out.println(mmtcmans);
			}
			if(mmwdyw.equalsIgnoreCase("m")) {
				          	System.out.print("write ne number of millimetres you want to convert to metres:  ");
							double mmtm = Integer.parseInt(readInput());
							double mmtmans = (mmtm / 10 / 100);
							System.out.print("this is the answer:  ");
				System.out.println(mmtmans);
			}
				if(mmwdyw.equalsIgnoreCase("km")) {
							          	System.out.print("write ne number of millimetres you want to convert to kilometres:  ");
										double mmtkm = Integer.parseInt(readInput());
										double mmtkmans = (mmtkm / 10 / 100 / 1000);
										System.out.print("this is the answer:  ");
				System.out.println(mmtkmans);
			}
            }
	if(wdyw.equalsIgnoreCase("cm")) {
	System.out.println("what do you want to convert it to? (mm m km)");
	          String cmwdyw = readInput();
	if(cmwdyw.equalsIgnoreCase("mm")) {
		System.out.print("write ne number of centermetres you want to convert to millimetres:  ");
		double cmtmm = Integer.parseInt(readInput());
		double cmtmmans = (cmtmm * 100);
		System.out.print("this is the answer:  ");
		System.out.println(cmtmmans);
}
	if(cmwdyw.equalsIgnoreCase("m")) {
		System.out.print("write ne number of centermetres you want to convert to metres:  ");
		double cmtm = Integer.parseInt(readInput());
		double cmtmans = (cmtm / 1000);
		System.out.print("this is the answer:  ");
		System.out.println(cmtmans);
}
	if(cmwdyw.equalsIgnoreCase("km")) {
	System.out.print("write ne number of centermetres you want to convert to kilometres:  ");
	double cmtkm = Integer.parseInt(readInput());
	double cmtkmans = (cmtkm / 100 / 1000);
	System.out.print("this is the answer:  ");
	System.out.println(cmtkmans);
}
}
if(wdyw.equalsIgnoreCase("m")) {
		System.out.println("what do you want to convert it to? (mm cm km)");
		          String mwdyw = readInput();
			if(mwdyw.equalsIgnoreCase("mm")) {
	           System.out.print("write ne number of metres you want to convert to millimetres:  ");
			  	double mtmm = Integer.parseInt(readInput());
			  	double mtmmans = (mtmm * 100 * 10);
			  	System.out.print("this is the answer:  ");
			  	System.out.println(mtmmans);
            }
	          if(mwdyw.equalsIgnoreCase("cm")) {
			  	           System.out.print("write ne number of metres you want to convert to centermetres:  ");
			  			  	double mtcm = Integer.parseInt(readInput());
			  			  	double mtcmans = (mtcm * 100);
			  			  	System.out.print("this is the answer:  ");
			  			  	System.out.println(mtcmans);
            }
	          if(mwdyw.equalsIgnoreCase("km")) {
	          System.out.print("write ne number of metres you want to convert to kilometres:  ");
	double mtkm = Integer.parseInt(readInput());
	double mtkmans = (mtkm / 1000);
	System.out.print("this is the answer:  ");
	System.out.println(mtkmans);
            }
		}
            if(wdyw.equalsIgnoreCase("km")) {
            System.out.println("what do you want to convert it to? (mm cm m)");
            String kmwdyw = readInput();
            if(kmwdyw.equalsIgnoreCase("km")) {
				          System.out.print("write ne number of kilometres you want to convert to milimetres:  ");
				double kmtmm = Integer.parseInt(readInput());
				double kmtmmans = (kmtmm * 1000 * 100 * 10);
				System.out.print("this is the answer:  ");
				System.out.println(kmtmmans);
			}
			if(kmwdyw.equalsIgnoreCase("cm")) {
							          System.out.print("write ne number of kilometres you want to convert to centermetres:  ");
							double kmtcm = Integer.parseInt(readInput());
							double kmtcmans = (kmtcm * 1000 * 100);
							System.out.print("this is the answer:  ");
							System.out.println(kmtcmans);
						}
						if(kmwdyw.equalsIgnoreCase("m")) {
													          System.out.print("write ne number of kilometres you want to convert to metres:  ");
													double kmtm = Integer.parseInt(readInput());
													double kmtmans = (kmtm * 1000);
													System.out.print("this is the answer:  ");
													System.out.println(kmtmans);

						}
            }
            System.out.println("\n\n\n");

		}while(hh == 1);
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