package gtu.cse.cse396.sdbelt.scan.infra.adapter;

import java.time.LocalDateTime;
import java.time.ZoneOffset;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.stream.Collectors;

import org.springframework.data.domain.Page;
import org.springframework.data.domain.PageRequest;
import org.springframework.data.domain.Pageable;
import org.springframework.data.domain.Sort;
import org.springframework.data.jpa.domain.Specification;
import org.springframework.stereotype.Component;
import org.springframework.transaction.annotation.Transactional;

import gtu.cse.cse396.sdbelt.scan.domain.model.GeneralStatistics;
import gtu.cse.cse396.sdbelt.scan.domain.model.ProductStatistics;
import gtu.cse.cse396.sdbelt.scan.domain.model.Scan;
import gtu.cse.cse396.sdbelt.scan.domain.service.ScanService;
import gtu.cse.cse396.sdbelt.scan.infra.mapper.ScanMapper;
import gtu.cse.cse396.sdbelt.shared.model.FilterTime;
import gtu.cse.cse396.sdbelt.scan.infra.model.ScanEntity;
import gtu.cse.cse396.sdbelt.scan.infra.model.ScanFilter;
import gtu.cse.cse396.sdbelt.scan.infra.repository.JpaScanRepository;
import lombok.RequiredArgsConstructor;

/**
 * Adapter implementation for Scan domain operations.
 * This class implements the ScanService interface and provides
 * concrete implementations using JPA repository.
 */
@Component
@RequiredArgsConstructor
@Transactional
public class ScanAdapter implements ScanService {

    private final JpaScanRepository scanRepository;

    /**
     * Creates a new scan record.
     *
     * @param productId    The unique identifier of the product that was scanned
     * @param isSuccess    Indicates whether the scan was successful
     * @param errorMessage A descriptive message if the scan failed; null otherwise
     */
    @Override
    @Transactional
    public void create(UUID productId, Boolean isSuccess, String errorMessage) {
        Scan scan = Scan.builder()
                .productId(productId)
                .isSuccess(isSuccess)
                .errorMessage(errorMessage)
                .timestamp(LocalDateTime.now().toEpochSecond(ZoneOffset.UTC))
                .build();
        scanRepository.save(ScanMapper.toEntity(scan));
    }

    /**
     * Retrieves all scan records.
     *
     * @return A list of all scan records
     */
    @Override
    @Transactional(readOnly = true)
    public List<Scan> list() {
        return scanRepository.findAll().stream()
                .map(ScanMapper::toDomain)
                .toList();
    }

    /**
     * Finds scan records based on the provided filter.
     *
     * @param filter The filter criteria to apply
     * @return A list of scan records matching the filter
     */
    @Override
    @Transactional(readOnly = true)
    public List<Scan> find(ScanFilter filter) {
        if (filter == null) {
            return list();
        }

        // Create sorting options if provided
        Sort sort = Sort.unsorted();
        if (filter.sort() != null && !filter.sort().isEmpty()) {
            List<Sort.Order> orders = filter.sort().stream()
                    .map(sortStr -> {
                        if (sortStr.startsWith("-")) {
                            return Sort.Order.desc(sortStr.substring(1));
                        } else {
                            return Sort.Order.asc(sortStr);
                        }
                    })
                    .toList();
            sort = Sort.by(orders);
        }

        // Create pageable if pagination parameters are provided
        Pageable pageable = Pageable.unpaged();
        if (filter.page() != null && filter.size() != null) {
            pageable = PageRequest.of(filter.page(), filter.size(), sort);
        } else if (!sort.isEmpty()) {
            pageable = PageRequest.of(0, Integer.MAX_VALUE, sort);
        }

        // Build dynamic query based on filter criteria
        Specification<ScanEntity> spec = Specification.where(null);
        
        // Add product ID filter
        if (filter.productId() != null) {
            spec = spec.and((root, query, cb) -> cb.equal(root.get("productId"), filter.productId()));
        }
        
        // Add success status filter
        if (filter.isSuccess() != null) {
            spec = spec.and((root, query, cb) -> cb.equal(root.get("isSuccess"), filter.isSuccess()));
        }
        
        // Add time filter if present
        if (filter.time() != null) {
            FilterTime timeFilter = filter.time();
            
            // Parse string time values to LocalDateTime if not empty
            if (timeFilter.startTime() != null && !timeFilter.startTime().isEmpty() && 
                timeFilter.endTime() != null && !timeFilter.endTime().isEmpty()) {
                
                try {
                    // Parse ISO format datetime strings (adjust the parsing format if needed)
                    LocalDateTime startTime = LocalDateTime.parse(timeFilter.startTime());
                    LocalDateTime endTime = LocalDateTime.parse(timeFilter.endTime());
                    
                    spec = spec.and((root, query, cb) -> 
                        cb.between(root.get("timestamp"), startTime, endTime));
                } catch (Exception e) {
                    // Log the parsing error and continue without time filtering
                    System.err.println("Error parsing time filter: " + e.getMessage());
                }
            }
        }
        
        // Execute query
        Page<ScanEntity> page = scanRepository.findAll(spec, pageable);
        
        // Map entities to domain objects
        return page.getContent().stream()
                .map(ScanMapper::toDomain)
                .toList();
    }

    /**
     * Generates general statistics for all scans within the specified date range.
     *
     * @param startDate The beginning of the date range
     * @param endDate   The end of the date range
     * @return General statistics for the specified period
     */
    @Override
    @Transactional(readOnly = true)
    public GeneralStatistics generateStatistics(LocalDateTime startDate, LocalDateTime endDate) {
        List<ScanEntity> scans = scanRepository.findByTimestampBetween(startDate, endDate);
        
        long totalScanned = scans.size();
        long totalSuccess = scans.stream().filter(ScanEntity::getIsSuccess).count();
        long totalFailed = totalScanned - totalSuccess;
        
        // Calculate success and failure rates
        double successRate = totalScanned > 0 ? (double) totalSuccess / totalScanned * 100.0 : 0.0;
        double failureRate = totalScanned > 0 ? (double) totalFailed / totalScanned * 100.0 : 0.0;
        
        // Group scans by product ID for product-specific statistics
        Map<UUID, List<ScanEntity>> scansByProduct = scans.stream()
                .collect(Collectors.groupingBy(ScanEntity::getProductId));
        
        List<ProductStatistics> productStatistics = new ArrayList<>();
        
        scansByProduct.forEach((productId, productScans) -> {
            productStatistics.add(generateProductStatistics(productId, startDate, endDate, productScans));
        });
        
        return new GeneralStatistics(
                startDate,
                endDate,
                totalScanned,
                totalSuccess,
                totalFailed,
                successRate,
                failureRate,
                productStatistics
        );
    }

    /**
     * Generates statistics for a specific product within the specified date range.
     *
     * @param productId  The unique identifier of the product
     * @param startDate  The beginning of the date range
     * @param endDate    The end of the date range
     * @return Product-specific statistics for the specified period
     */
    @Override
    @Transactional(readOnly = true)
    public ProductStatistics generateStatistics(UUID productId, LocalDateTime startDate, LocalDateTime endDate) {
        List<ScanEntity> scans = scanRepository.findByProductIdAndTimestampBetween(productId, startDate, endDate);
        return generateProductStatistics(productId, startDate, endDate, scans);
    }
    
    /**
     * Helper method to generate product statistics from a list of scan entities.
     * 
     * @param productId  The unique identifier of the product
     * @param startDate  The beginning of the date range
     * @param endDate    The end of the date range
     * @param scans      The list of scan entities for this product
     * @return Product-specific statistics
     */
    private ProductStatistics generateProductStatistics(UUID productId, LocalDateTime startDate, LocalDateTime endDate, List<ScanEntity> scans) {
        long totalScanned = scans.size();
        long totalSuccess = scans.stream().filter(ScanEntity::getIsSuccess).count();
        long totalFailed = totalScanned - totalSuccess;
        
        double successRate = totalScanned > 0 ? (double) totalSuccess / totalScanned * 100.0 : 0.0;
        double failureRate = totalScanned > 0 ? (double) totalFailed / totalScanned * 100.0 : 0.0;
        
        return new ProductStatistics(
                startDate,
                endDate,
                productId,
                totalScanned,
                totalSuccess,
                totalFailed,
                successRate,
                failureRate
        );
    }
}