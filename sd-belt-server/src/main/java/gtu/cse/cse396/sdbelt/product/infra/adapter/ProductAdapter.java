package gtu.cse.cse396.sdbelt.product.infra.adapter;

import java.time.LocalDateTime;
import java.util.List;
import java.util.Optional;
import java.util.UUID;

import org.springframework.stereotype.Component;

import gtu.cse.cse396.sdbelt.product.domain.model.Product;
import gtu.cse.cse396.sdbelt.product.domain.service.ProductService;
import gtu.cse.cse396.sdbelt.product.infra.mapper.ProductMapper;
import gtu.cse.cse396.sdbelt.product.infra.model.ProductEntity;
import gtu.cse.cse396.sdbelt.product.infra.repository.JpaProductRepository;
import lombok.RequiredArgsConstructor;

@Component
@RequiredArgsConstructor
public class ProductAdapter implements ProductService {

    private final JpaProductRepository productRepository;

    @Override
    public void create(String name, String description) {
         Product product = Product.builder().id(UUID.randomUUID()).name(name).description(description)
                .createdAt(LocalDateTime.now()).updatedAt(LocalDateTime.now()).build();
        ProductEntity entity = ProductMapper.toEntity(product);
        productRepository.save(entity);
    }

    @Override
    public void update(UUID id, String name, String description) {
        ProductEntity entity = productRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Product not found with id: " + id));
        entity.setName(name);
        entity.setDescription(description);
        entity.setUpdatedAt(LocalDateTime.now());
        productRepository.save(entity);
    }

    @Override
    public void delete(UUID id) {
        productRepository.deleteById(id);
    }

    @Override
    public Product get(UUID id) {
        ProductEntity entity = productRepository.findById(id)
                .orElseThrow(() -> new IllegalArgumentException("Product not found with id: " + id));
        return ProductMapper.toDomain(entity);
    }

    @Override
    public Optional<Product> find(UUID id) {
        if(productRepository.existsById(id)) {
            ProductEntity entity = productRepository.findById(id).orElse(null);
            return Optional.of(ProductMapper.toDomain(entity));
        } else {
            return Optional.empty();
        }
    }

    @Override
    public List<Product> list() {
        List<ProductEntity> entities = productRepository.findAll();
        return entities.stream().map(ProductMapper::toDomain).toList();
    }
}
