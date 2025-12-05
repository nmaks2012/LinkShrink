import pytest
import time

async def test_full_lifecycle(service_client):
    
    urls_to_test = [
        'https://yandex.ru',
        'https://google.com',
        'https://userver.tech/docs/',
    ]
    short_codes_map = {}

    for original_url in urls_to_test:
        response = await service_client.post(
            '/shorten',
            json={'url': original_url},
        )
        assert response.status == 200
        data = response.json()
        assert 'short_url' in data

        short_url = data['short_url']
        short_code = short_url.split('/')[-1]

        short_codes_map[original_url] = short_code
        print(f"Created link: {original_url} -> {short_code}")

    assert len(short_codes_map) == len(urls_to_test)

    for i, (original_url, short_code) in enumerate(short_codes_map.items()):
        clicks_to_make = i + 1
        for _ in range(clicks_to_make):
            response = await service_client.get(
                f'/{short_code}',
                allow_redirects=False,
            )
            assert response.status == 302
            assert response.headers['Location'] == original_url

    time.sleep(10)
    
    for i, (original_url, short_code) in enumerate(short_codes_map.items()):
        expected_clicks = i + 1        
        response = await service_client.get(f'/stats/{short_code}')
        assert response.status == 200
        stats = response.json()

        assert stats['original_url'] == original_url
        assert len(stats['clicks']) == expected_clicks

        first_click = stats['clicks'][0]
        assert first_click['http_method'] == 'GET'
        assert first_click['trace_id'] is not None

async def test_invalid_short_code(service_client):
    
    response_redirect = await service_client.get(
        '/this-code-does-not-exist',
        allow_redirects=False,
    )
    assert response_redirect.status == 404

    response_stats = await service_client.get('/stats/this-code-does-not-exist')
    assert response_stats.status == 404