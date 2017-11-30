
public class EmptyDicomException extends Exception {

    public EmptyDicomException(String reason) {
	super(reason);
    }

    public EmptyDicomException() {
	super();
    }

}

