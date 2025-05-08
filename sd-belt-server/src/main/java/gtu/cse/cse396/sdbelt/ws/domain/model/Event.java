package gtu.cse.cse396.sdbelt.ws.domain.model;

public interface Event<T> {
    EventType type();
    T data();
    long timestamp();
}
