public class Process {
  public final int id;
  public final int cpuTime;
  public final int ioBlockIn;
  public final Type type;
  public int cpuDone = 0;
  public int numBlocked = 0;

  public enum Type {
    System(0),
    Background(1),
    Foreground(2);

    private final int value;
    private Type(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }
  };

  public Process(int id, int cpuTime, int ioBlockIn, Type type) {
    this.id = id;
    this.cpuTime = cpuTime;
    this.ioBlockIn = ioBlockIn;
    this.type = type;
  }
}
