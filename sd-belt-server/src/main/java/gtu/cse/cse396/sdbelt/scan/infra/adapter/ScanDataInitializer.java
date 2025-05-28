package gtu.cse.cse396.sdbelt.scan.infra.adapter;

import gtu.cse.cse396.sdbelt.scan.domain.service.ScanService;
import lombok.RequiredArgsConstructor;
import org.springframework.boot.ApplicationArguments;
import org.springframework.boot.ApplicationRunner;
import org.springframework.stereotype.Component;

import java.time.Instant;
import java.time.LocalDateTime;
import java.time.ZoneOffset;
import java.util.Random;
import java.util.concurrent.ThreadLocalRandom;

@Component
@RequiredArgsConstructor
public class ScanDataInitializer implements ApplicationRunner {

    private final ScanService scanService;

    private static final int SCAN_COUNT = 1000;

    @Override
    public void run(ApplicationArguments args) {
        if (scanService.list().size() >= SCAN_COUNT) {
            System.out.println("Scan data already initialized.");
            return;
        }

        System.out.println("Initializing scan data...");

        final String[] PRODUCT_IDS = { "BANANA", "APPLE" };

        Random random = new Random();

        for (int i = 0; i < SCAN_COUNT; i++) {
            String productId = PRODUCT_IDS[random.nextInt(PRODUCT_IDS.length)];
            double threshold = 80.0;
            Double healthRatio = random.nextDouble(100);
            boolean isSuccess = healthRatio > threshold;
            String errorMessage = isSuccess ? null : "Scan failed due to random error";

            // Generate random timestamp between 2 years ago and now
            LocalDateTime now = LocalDateTime.now();
            LocalDateTime sixDaysAgo = now.minusDays(6);

            long randomEpoch = ThreadLocalRandom.current()
                    .nextLong(sixDaysAgo.toEpochSecond(ZoneOffset.UTC), now.toEpochSecond(ZoneOffset.UTC));

            LocalDateTime randomTimestamp = LocalDateTime.ofInstant(Instant.ofEpochSecond(randomEpoch), ZoneOffset.UTC);

            // Save scan
            scanService.create(productId, random.nextDouble(100), isSuccess, errorMessage, randomTimestamp);

            // Optionally print progress every 100
            if (i % 100 == 0) {
                System.out.println("Inserted " + i + " scans...");
            }
        }

        System.out.println("Scan data initialization complete.");
    }
}
