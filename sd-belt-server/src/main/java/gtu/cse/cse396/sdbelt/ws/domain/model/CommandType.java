package gtu.cse.cse396.sdbelt.ws.domain.model;

public enum CommandType {
    START_SYSTEM_COMMAND("START_SYSTEM_COMMAND"),
    STOP_SYSTEM_COMMAND("STOP_SYSTEM_COMMAND"),
    RESTART_SYSTEM_COMMAND("RESTART_SYSTEM_COMMAND"),
    SHUTDOWN_SYSTEM_COMMAND("SHUTDOWN_SYSTEM_COMMAND"),
    REVERSE_BELT_COMMAND("REVERSE_BELT_COMMAND"),
    ;

    private final String name;

    CommandType(String name) {
        this.name = name;
    }
}
