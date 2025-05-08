package gtu.cse.cse396.sdbelt.system.infra.model;

import java.util.UUID;

import gtu.cse.cse396.sdbelt.system.domain.model.SystemStatus;
import jakarta.persistence.*;
import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.time.LocalDateTime;

/**
 * JPA entity representing the system record in the conveyor belt systemning system.
 * <p>
 * Each instance corresponds to a row in the {@code systems} table and captures the
 * result of an individual system operation for a product.
 */
@Entity
@Table(name = "systems")
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
public class SystemEntity {

    @Id
    @Column(columnDefinition = "VARCHAR(36)") // H2 workaround for UUID
    private UUID id;

    @Column(nullable = false)
    private String name;

    private String description;

    @Column(name = "created_at", updatable = false)
    private LocalDateTime createdAt;

    @Column(name = "run_at")
    private LocalDateTime runAt;

    @PrePersist
    protected void onCreate() {
        this.createdAt = LocalDateTime.now();
        this.runAt = LocalDateTime.now();
    }

    @Enumerated(EnumType.STRING)
    @Column(name = "status", nullable = false)
    private SystemStatus status;

    @Column(name = "accuracy")
    private Integer accuracy;

    @Column(name = "speed")
    private Integer speed;
}
