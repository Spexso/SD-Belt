package gtu.cse.cse396.sdbelt.ws.domain.model;

public enum MessageType {
    EVENT("EVENT"),
    COMMAND("COMMAND"),
    INFO("INFO");

    private final String type;

    MessageType(String type) {
        this.type = type;
    }

    public String getType() {
        return type;
    }
}
