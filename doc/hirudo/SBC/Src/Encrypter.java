// CIPHER / GENERATORS
import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.KeyGenerator;

// KEY SPECIFICATIONS
import java.security.spec.KeySpec;
import java.security.spec.AlgorithmParameterSpec;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEParameterSpec;

// EXCEPTIONS
import java.security.InvalidAlgorithmParameterException;
import java.security.NoSuchAlgorithmException;
import java.security.InvalidKeyException;
import java.security.spec.InvalidKeySpecException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.BadPaddingException;
import javax.crypto.IllegalBlockSizeException;
import java.io.UnsupportedEncodingException;
import java.io.IOException;

public class Encrypter{
	String key;

	public Encrypter(String key){
		this.key = key;


	}
	public void setKey(String key){
		this.key = key;
	}

	public String encrypt(String str){
		 String secretString = str;
		 String passPhrase   = key;
		 StringEncrypter desEncrypter = new StringEncrypter(passPhrase);

		 // Encrypt the string
		 String desEncrypted = desEncrypter.encrypt(secretString);
		 return desEncrypted;

	}
	public String decrypt(String str){
		 //String secretString = str;
		 String passPhrase   = key;
		 StringEncrypter desEncrypter = new StringEncrypter(passPhrase);
		 String desDecrypted = desEncrypter.decrypt(str);
		 return desDecrypted;
	}
}