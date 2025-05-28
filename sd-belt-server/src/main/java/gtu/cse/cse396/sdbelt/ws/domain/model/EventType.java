package gtu.cse.cse396.sdbelt.ws.domain.model;

public enum EventType {
    NEW_SCAN_EVENT("NEW_SCAN_EVENT"),
    STATUS_EVENT("STATUS_EVENT"),
    ERROR_EVENT("ERROR_EVENT"),
    WARNING_EVENT("WARNING_EVENT"),
    INITIALIZE_EVENT("INITIALIZE_EVENT"),
    ;

    private final String name;

    EventType(String name) {
        this.name = name;
    }
}
