package gtu.cse.cse396.sdbelt.scan.infra.handler;

import com.fasterxml.jackson.databind.ObjectMapper;

import gtu.cse.cse396.sdbelt.scan.domain.model.Scan;
import gtu.cse.cse396.sdbelt.scan.infra.mapper.ScanMapper;
import gtu.cse.cse396.sdbelt.scan.infra.model.ScanEntity;
import gtu.cse.cse396.sdbelt.scan.infra.repository.JpaScanRepository;
import gtu.cse.cse396.sdbelt.ws.domain.model.Event;
import gtu.cse.cse396.sdbelt.ws.domain.model.EventHandler;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;

@RequiredArgsConstructor
@Slf4j
public class NewScanEventHandler implements EventHandler<Scan> {

    private final JpaScanRepository scanRepository;
    private final ObjectMapper objectMapper;

    @Override
    public void handle(Event<Scan> event) {
        // Convert the RawEvent to a Scan object
        Scan scan = objectMapper.convertValue(event.data(), Scan.class);
        // Save the scan to the database
        ScanEntity entity = ScanMapper.toEntity(scan);
        scanRepository.save(entity);
    }

}
