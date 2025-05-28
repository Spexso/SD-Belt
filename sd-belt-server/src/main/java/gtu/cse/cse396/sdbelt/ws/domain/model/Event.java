package gtu.cse.cse396.sdbelt.ws.domain.model;

public interface Event<T> extends MessageContent {
    EventType type();

    T data();
}
