package gtu.cse.cse396.sdbelt.ws.domain.model;

public enum EventType {
    NEW_SCAN_EVENT("NEW_SCAN_EVENT")
    ;

    private final String name;

    EventType(String name) {
        this.name = name;
    }
}
