package gtu.cse.cse396.sdbelt.scan.infra.model;

import jakarta.persistence.*;
import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;
import lombok.ToString;

import java.time.LocalDateTime;

/**
 * JPA entity representing a scan record in the conveyor belt scanning system.
 * <p>
 * Each instance corresponds to a row in the {@code scans} table and captures
 * the
 * result of an individual scan operation for a product.
 */
@Entity
@Table(name = "scans")
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
@ToString
public class ScanEntity {

    /**
     * The unique identifier for this scan record.
     * <p>
     * This field is auto-generated and serves as the primary key for the entity.
     */
    @Id
    @GeneratedValue
    private Long id;

    /**
     * The unique identifier of the product that was scanned.
     * <p>
     * This field acts as a foreign key reference to a {@code ProductEntity},
     * although not explicitly mapped here.
     */
    private String productId;

    /**
     * The timestamp when the scan was performed.
     * <p>
     * Automatically set just before the entity is persisted using
     * {@link #onCreate()}.
     */

    private Double healthRatio;

    /**
     * Indicates whether the scan was successful.
     * <p>
     * A value of {@code true} denotes a successful scan, while {@code false}
     * indicates a failure.
     */
    private Boolean isSuccess;

    /**
     * A descriptive message provided when a scan fails.
     * <p>
     * Should be {@code null} or empty if {@link #isSuccess} is {@code true}.
     */
    private String errorMessage;

    /**
     * JPA lifecycle callback used to automatically set the {@link #timestamp}
     * to the current system time just before the entity is persisted.
     */

    private LocalDateTime timestamp;
}
