package gtu.cse.cse396.sdbelt.scan.domain.model;

import gtu.cse.cse396.sdbelt.ws.domain.model.Event;
import gtu.cse.cse396.sdbelt.ws.domain.model.EventType;

public record ScanEvent(
        EventType type,
        Scan data,
        long timestamp) implements Event<Scan> {
}
