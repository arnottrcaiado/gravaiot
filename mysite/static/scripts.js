document.addEventListener('DOMContentLoaded', function() {
    let originalData = [];

    fetch('/mostraDados')
        .then(response => response.json())
        .then(data => {
            originalData = data;
            renderTable(data);
        })
        .catch(error => console.error('Erro ao buscar os dados:', error));

    const filters = document.querySelectorAll('input[type="text"]');
    filters.forEach(filter => {
        filter.addEventListener('input', () => {
            const filteredData = originalData.filter(item => {
                return Array.from(filters).every(input => {
                    const column = input.getAttribute('data-column');
                    return item[column].toString().includes(input.value);
                });
            });
            renderTable(filteredData);
        });
    });

    document.getElementById('reset-filters').addEventListener('click', () => {
        filters.forEach(filter => filter.value = '');
        renderTable(originalData);
    });

    function renderTable(data) {
        const tableBody = document.querySelector('#data-table tbody');
        tableBody.innerHTML = '';
        data.forEach(item => {
            const row = document.createElement('tr');
            row.innerHTML = `
                <td>${item.data}</td>
                <td>${item.hora}</td>
                <td>${item.id}</td>
                <td>${item.sensor}</td>
                <td>${item.valor}</td>
            `;
            tableBody.appendChild(row);
        });

        document.getElementById('row-count').textContent = `Total de Linhas: ${data.length}`;
    }
});
