package gtu.cse.cse396.sdbelt.system.infra.scheduler;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Service;

import gtu.cse.cse396.sdbelt.system.domain.model.SystemStatus;
import gtu.cse.cse396.sdbelt.system.domain.service.SystemService;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import java.net.InetAddress;

@Service
@Slf4j
@RequiredArgsConstructor
public class IcmpPingScheduler {

    @Value("${system.server.host}")
    private String host;

    private final SystemService systemService;

    @Scheduled(fixedDelay = 2000) // every 10 seconds
    public void pingServer() {
        try {
            InetAddress inet = InetAddress.getByName(host);

            // Timeout in milliseconds
            boolean reachable = inet.isReachable(2000);

            if (reachable) {
                log.info("Ping to {} successful", host);
                systemService.update(SystemStatus.ACTIVE);

            } else {
                log.warn("Ping to {} failed", host);
                systemService.update(SystemStatus.INACTIVE, 0.0, 0.0, 0.0);
            }
        } catch (Exception e) {
            log.debug("Exception during ping");
            systemService.update(SystemStatus.INACTIVE);
        }
    }
}
