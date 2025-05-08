package gtu.cse.cse396.sdbelt.ws.domain.model;

public interface EventHandler<T> {
    
    void handle(Event<T> event);
}
    
