
public class PTBR {

    public static String maybePlural(String word, int n) {
	if (n==1) return word;
	if (word.endsWith("m"))                       return( word.substring(0,word.length()-1) + "ns" );
	if (word.endsWith("z") || word.endsWith("r")) return( word+"es" );
	if (word.endsWith("ão"))                      return( word.substring(0,word.length()-2) + "ões" );
	if (word.endsWith("l"))                       return( word.substring(0,word.length()-1) + "is" );
	return(word+"s");
    }

}
