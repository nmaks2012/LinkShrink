document.addEventListener('DOMContentLoaded', () => {
    if (document.getElementById('shorten-form')) {
        initShortenPage();
    } else if (document.getElementById('stats-table')) {
        initStatsPage();
    }
});

// Logic for the main page (index.html)
function initShortenPage() {
    const form = document.getElementById('shorten-form');
    const urlInput = document.getElementById('url-input');
    const resultArea = document.getElementById('result-area');
    const errorArea = document.getElementById('error-area');
    const errorMessage = document.getElementById('error-message');
    const shortUrlLink = document.getElementById('short-url-link');
    const statsLink = document.getElementById('stats-link');
    const copyButton = document.getElementById('copy-button');
    const qrContainer = document.getElementById('qrcode-container');
    
    // History elements
    const historyArea = document.getElementById('history-area');
    const historyTbody = document.getElementById('history-tbody');
    const clearHistoryBtn = document.getElementById('clear-history-btn');

    // --- HISTORY FUNCTIONS ---

    // 1. Load and render history
    const renderHistory = () => {
        const history = JSON.parse(localStorage.getItem('linkHistory') || '[]');
        
        if (history.length === 0) {
            historyArea.classList.add('hidden');
            return;
        }

        historyTbody.innerHTML = ''; // Clear table
        history.forEach(item => {
            const row = document.createElement('tr');
            
            // Original URL (truncate if long)
            const originalCell = document.createElement('td');
            originalCell.textContent = item.original.length > 40 ? item.original.substring(0, 40) + '...' : item.original;
            originalCell.title = item.original;
            
            // Short URL
            const shortCell = document.createElement('td');
            const link = document.createElement('a');
            link.href = item.short;
            link.target = '_blank';
            link.textContent = item.short;
            link.style.color = 'var(--accent)';
            shortCell.appendChild(link);

            // Actions (Stats)
            const actionsCell = document.createElement('td');
            const shortCode = item.short.split('/').pop();
            const statsBtn = document.createElement('a');
            statsBtn.href = `/static/stats.html?code=${shortCode}`;
            statsBtn.textContent = '📊 Statistics';
            statsBtn.style.textDecoration = 'none';
            statsBtn.style.fontSize = '0.9rem';
            statsBtn.style.color = 'var(--text-muted)';
            actionsCell.appendChild(statsBtn);

            row.appendChild(originalCell);
            row.appendChild(shortCell);
            row.appendChild(actionsCell);
            historyTbody.appendChild(row);
        });

        historyArea.classList.remove('hidden');
    };

    // 2. Add new entry to history
    const addToHistory = (original, short) => {
        let history = JSON.parse(localStorage.getItem('linkHistory') || '[]');
        
        // Check for duplicates
        const exists = history.some(h => h.short === short);
        if (!exists) {
            // Add to the beginning
            history.unshift({ original, short, date: Date.now() });
            
            // Limit history size (e.g., keep last 10)
            if (history.length > 10) {
                history = history.slice(0, 10);
            }
            
            localStorage.setItem('linkHistory', JSON.stringify(history));
            renderHistory();
        }
    };

    // 3. Clear history
    clearHistoryBtn.addEventListener('click', () => {
        if(confirm('Are you sure you want to clear history?')) {
            localStorage.removeItem('linkHistory');
            renderHistory();
        }
    });

    // --- MAIN LOGIC ---

    const renderResult = (shortUrl, originalUrl) => {
        const shortCode = shortUrl.split('/').pop();

        shortUrlLink.href = shortUrl;
        shortUrlLink.textContent = shortUrl;
        statsLink.href = `/static/stats.html?code=${shortCode}`;
        
        if (originalUrl) {
            urlInput.value = originalUrl;
        }

        if (qrContainer) {
            qrContainer.innerHTML = '';
            new QRCode(qrContainer, {
                text: shortUrl,
                width: 128,
                height: 128,
                colorDark : "#000000",
                colorLight : "#ffffff",
                correctLevel : QRCode.CorrectLevel.H
            });
        }

        resultArea.classList.remove('hidden');
        errorArea.classList.add('hidden');
    };

    // Render history on load
    renderHistory();

    // Check for last created link (for F5 recovery)
    const lastShort = localStorage.getItem('lastShortUrl');
    const lastOriginal = localStorage.getItem('lastOriginalUrl');
    if (lastShort) {
        renderResult(lastShort, lastOriginal);
    }

    form.addEventListener('submit', async (event) => {
        event.preventDefault();
        resultArea.classList.add('hidden');
        errorArea.classList.add('hidden');

        const longUrl = urlInput.value;

        try {
            const response = await fetch('/shorten', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ url: longUrl }),
            });

            if (!response.ok) {
                throw new Error(`Server error: ${response.statusText}`);
            }

            const data = await response.json();

            // Save current link
            localStorage.setItem('lastShortUrl', data.short_url);
            localStorage.setItem('lastOriginalUrl', longUrl);

            // Add to history
            addToHistory(longUrl, data.short_url);

            // Render result
            renderResult(data.short_url, longUrl);

        } catch (error) {
            errorMessage.textContent = error.message;
            errorArea.classList.remove('hidden');
        }
    });

    copyButton.addEventListener('click', () => {
        navigator.clipboard.writeText(shortUrlLink.href).then(() => {
            copyButton.textContent = 'Copied!';
            setTimeout(() => { copyButton.textContent = 'Copy'; }, 2000);
        });
    });
}

// Logic for the statistics page (stats.html)
async function initStatsPage() {
    const originalUrlEl = document.getElementById('original-url');
    const statsTbody = document.getElementById('stats-tbody');
    const loadingIndicator = document.getElementById('loading-indicator');
    const errorArea = document.getElementById('error-area');
    const errorMessage = document.getElementById('error-message');

    const params = new URLSearchParams(window.location.search);
    const shortCode = params.get('code');

    if (!shortCode) {
        errorMessage.textContent = 'Short code not found in URL.';
        errorArea.classList.remove('hidden');
        loadingIndicator.classList.add('hidden');
        return;
    }

    try {
        const response = await fetch(`/stats/${shortCode}`);
        if (!response.ok) {
            throw new Error('Link not found or error occurred.');
        }
        const data = await response.json();

        originalUrlEl.textContent = data.original_url;
        
        if (data.clicks && data.clicks.length > 0) {
            data.clicks.forEach(click => {
                const row = document.createElement('tr');
    
                const timeCell = document.createElement('td');
                timeCell.textContent = new Date(click.timestamp).toLocaleString();
                
                const methodCell = document.createElement('td');
                methodCell.textContent = click.http_method;
                methodCell.style.fontWeight = 'bold';
                methodCell.style.color = click.http_method === 'GET' ? '#4caf50' : '#ff9800';

                const platformCell = document.createElement('td');
                let platformText = click.platform ? click.platform.replace(/"/g, '') : 'Unknown';
                if (click.is_mobile) platformText += ' 📱';
                else platformText += ' 💻';
                platformCell.textContent = platformText;

                const countryCell = document.createElement('td');
                countryCell.textContent = click.country_code ? click.country_code : '—';

                const ipCell = document.createElement('td');
                ipCell.textContent = click.ip_address;

                const refererCell = document.createElement('td');
                refererCell.textContent = click.referer ? click.referer.substring(0, 20) + '...' : '—';
                if (click.referer) refererCell.title = click.referer;

                const traceCell = document.createElement('td');
                traceCell.textContent = click.trace_id.substring(0, 8) + '...';
                traceCell.title = click.trace_id;
                traceCell.style.fontFamily = 'monospace';
                traceCell.style.fontSize = '0.8em';
                traceCell.style.color = '#666';

                row.appendChild(timeCell);
                row.appendChild(methodCell);
                row.appendChild(platformCell);
                row.appendChild(countryCell);
                row.appendChild(ipCell);
                row.appendChild(refererCell);
                row.appendChild(traceCell);
                
                statsTbody.appendChild(row);
            });
        } else {
            const row = document.createElement('tr');
            const cell = document.createElement('td');
            cell.colSpan = 7; // Updated colspan to match header count
            cell.textContent = 'No clicks for this link yet.';
            row.appendChild(cell);
            statsTbody.appendChild(row);
        }

    } catch (error) {
        errorMessage.textContent = error.message;
        errorArea.classList.remove('hidden');
    } finally {
        loadingIndicator.classList.add('hidden');
    }
}